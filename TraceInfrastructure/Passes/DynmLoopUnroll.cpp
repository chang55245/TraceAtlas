#include "Passes/CommandArgs.h"
#include "Passes/Annotate.h"
#include "AtlasUtil/Annotate.h"
#include <cstdint>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Value.h>
#include <llvm/InitializePasses.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar/SimpleLoopUnswitch.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Use.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>




#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
        struct LoopProfileInfo {
        uint64_t count;
        uint64_t parentId;
    };

    class DynmLoopUnrollPass : public FunctionPass {
    private:
        std::map<uint64_t, LoopProfileInfo> loopIteration;
        UnrollLoopOptions ULO;
        
        // Analysis info
        DominatorTree* DT;
        LoopInfo* LI;
        AssumptionCache* AC;
        ScalarEvolution* SE;
        OptimizationRemarkEmitter* ORE;
        TargetLibraryInfo* TLI;

        // Store original conditions for potential restoration
        std::map<Loop*, Value*> originalConditions;

        void initializeUnrollOptions();
        void initializeAnalysis(Function &F);
        void cleanupAnalysis();
        std::map<Loop*, uint64_t> findLoopsWithTraces(Function &F);
        bool modifyLoopBound(Loop* L, uint64_t count);
        bool processLoop(Loop* L, uint64_t loopId);
        void collectLoopsPostOrder(Loop* L, 
                                 const std::map<Loop*, uint64_t>& loopMap,
                                 std::vector<std::pair<Loop*, uint64_t>>& loopsToProcess);
        void removeLoopTrace(Loop* L);
        void cleanupTraceCalls(Function& F);
        bool processNestedLoops(Loop* L, uint64_t loopId, map<Loop*, uint64_t>& loopMap);
        bool verifyLoopStructure(Loop* L);


    public:
        static char ID;
        DynmLoopUnrollPass() : FunctionPass(ID) {}
        bool runOnFunction(Function &F) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool doInitialization(Module &M) override;
    };
// namespace DashTracer::Passes

        void DynmLoopUnrollPass::initializeUnrollOptions() {
            ULO.Count = 5;
            ULO.TripCount = ULO.Count;
            ULO.Force = false;
            ULO.AllowRuntime = false;
            ULO.AllowExpensiveTripCount = false;
            ULO.PreserveCondBr = false;
            ULO.PreserveOnlyFirst = false;
            ULO.TripMultiple = 0;
            ULO.PeelCount = ULO.Count;
            ULO.UnrollRemainder = true;
            ULO.ForgetAllSCEV = true;
        }

        void DynmLoopUnrollPass::initializeAnalysis(Function &F) {
            DT = new DominatorTree(F);
            LI = new LoopInfo(*DT);
            AC = new AssumptionCache(F);
            TargetLibraryInfoImpl* TLII = new TargetLibraryInfoImpl();
            TLI = new TargetLibraryInfo(*TLII);
            SE = new ScalarEvolution(F, *TLI, *AC, *DT, *LI);
            ORE = new OptimizationRemarkEmitter(&F);
        }

        void DynmLoopUnrollPass::cleanupAnalysis() {
            delete DT;
            delete LI;
            delete AC;
            delete TLI;
            delete SE;
            delete ORE;
        }

        bool DynmLoopUnrollPass::verifyLoopStructure(Loop* L) {
            if (!L->isLoopSimplifyForm() || !L->isLCSSAForm(*DT)) {
                return false;
            }

            for (Loop* SubL : L->getSubLoops()) {
                if (!verifyLoopStructure(SubL)) {
                    return false;
                }
            }
            return true;
        }

        // Find all loops and their LoopTrace calls
        map<Loop*, uint64_t> DynmLoopUnrollPass::findLoopsWithTraces(Function &F) {
            map<Loop*, uint64_t> loopMap;
            for (auto &BB : F) {
                if (Loop* L = LI->getLoopFor(&BB)) {
                    for (auto &I : BB) {
                        if (auto *CI = dyn_cast<CallInst>(&I)) {
                            if (CI->getCalledFunction() && 
                                CI->getCalledFunction()->getName() == "LoopTrace") {
                                auto arg = CI->arg_begin();
                                if (auto* cons = dyn_cast<ConstantInt>(arg->get())) {
                                    uint64_t loopId = cons->getSExtValue();
                                    loopMap[L] = loopId;
                                }
                            }
                        }
                    }
                }
            }
            return loopMap;
        }

        // Modify loop bound for unrolling
        bool DynmLoopUnrollPass::modifyLoopBound(Loop* L, uint64_t count) {
            BasicBlock* Header = L->getHeader();
            
            for (auto& I : *Header) {
                if (auto *CI = dyn_cast<CmpInst>(&I)) {
                    if (CI == Header->getTerminator()->getOperand(0)) {
                        // Store original condition
                        originalConditions[L] = CI->clone();
                        
                        Value* IndVar = CI->getOperand(0);
                        Value* NewTripCount = ConstantInt::get(IndVar->getType(), count);
                        
                        IRBuilder<> Builder(Header);
                        Builder.SetInsertPoint(CI);
                        Value* NewCond = Builder.CreateICmpSLT(IndVar, NewTripCount);
                        
                        BranchInst* Term = cast<BranchInst>(Header->getTerminator());
                        BranchInst* NewTerm = BranchInst::Create(
                            Term->getSuccessor(0),
                            Term->getSuccessor(1),
                            NewCond
                        );
                        ReplaceInstWithInst(Term, NewTerm);
                        
                        // Keep original condition but make it inactive
                        CI->insertAfter(cast<Instruction>(NewCond));
                        CI->setOperand(0, UndefValue::get(CI->getType()));
                        return true;
                    }
                }
            }
            return false;
        }

        // Process a single loop
        bool DynmLoopUnrollPass::processLoop(Loop* L, uint64_t loopId) {
            if (!L || !L->getHeader()) return false;

            // Safety checks
            if (!verifyLoopStructure(L)) {
                return false;
            }

            ULO.Count = loopIteration[loopId].count;
            ULO.TripCount = ULO.Count;

            if (!modifyLoopBound(L, ULO.Count)) {
                return false;
            }

            bool PreserveLCSSA = true;
            Loop* ParentLoop = L->getParentLoop();
            
            // Perform the actual unrolling
            LoopUnrollResult Result = UnrollLoop(L, ULO, LI, SE, DT, AC, ORE, PreserveLCSSA);
            if (Result == LoopUnrollResult::Unmodified) {
                // Restore original condition if unrolling failed
                if (originalConditions.count(L)) {
                    // Restoration logic would go here
                    originalConditions.erase(L);
                }
                return false;
            }

            // Verify loop structure after unrolling
            if (!verifyLoopStructure(L)) {
                // Could add restoration logic here if needed
                return false;
            }

            removeLoopTrace(L);
            return true;
        }

        // Recursively process nested loops
        bool DynmLoopUnrollPass::processNestedLoops(Loop* L, uint64_t loopId, map<Loop*, uint64_t>& loopMap) {
            bool modified = false;

            // Build complete hierarchy map
            std::map<Loop*, std::vector<Loop*>> hierarchy;
            for (Loop* SubL : L->getSubLoops()) {
                if (loopMap.count(SubL)) {
                    uint64_t subLoopId = loopMap[SubL];
                    if (loopIteration[subLoopId].parentId == loopId) {
                        hierarchy[L].push_back(SubL);
                    }
                }
            }

            // Process loops in hierarchical order
            for (const auto& [parent, children] : hierarchy) {
                for (Loop* child : children) {
                    uint64_t childId = loopMap[child];
                    modified |= processLoop(child, childId);
                    
                    // Verify hierarchy is preserved
                    if (!child->getParentLoop() || child->getParentLoop() != parent) {
                        return false;
                    }
                }
                modified |= processLoop(parent, loopMap[parent]);
            }

            return modified;
        }


        bool DynmLoopUnrollPass::doInitialization(Module& M) {
            nlohmann::json j;
            std::ifstream inputStream(LoopTraceFile);
            inputStream >> j;
            inputStream.close();

            for (const auto& loop : j["loopIteration"]) {
                uint64_t loopId = loop["loopId"];
                loopIteration[loopId] = {
                    loop["count"],
                    loop["parentId"]
                };
            }
            return false;
        }

        bool DynmLoopUnrollPass::runOnFunction(Function &F) {
            if (F.empty()) return false;

            initializeUnrollOptions();
            initializeAnalysis(F);

            bool modified = false;
            auto loopMap = findLoopsWithTraces(F);

            // Process loops in post-order
            std::vector<std::pair<Loop*, uint64_t>> loopsToProcess;
            for (Loop* L : *LI) {
                collectLoopsPostOrder(L, loopMap, loopsToProcess);
            }

            // Process loops in the correct order
            for (const auto& [loop, loopId] : loopsToProcess) {
                if (loopIteration.find(loopId) != loopIteration.end()) {
                    modified |= processLoop(loop, loopId);
                }
            }

            cleanupTraceCalls(F);
            cleanupAnalysis();
            return modified;
        }

        void DynmLoopUnrollPass::collectLoopsPostOrder(
            Loop* L, 
            const std::map<Loop*, uint64_t>& loopMap,
            std::vector<std::pair<Loop*, uint64_t>>& loopsToProcess) 
        {
            for (Loop* SubL : L->getSubLoops()) {
                collectLoopsPostOrder(SubL, loopMap, loopsToProcess);
            }

            if (loopMap.count(L)) {
                uint64_t loopId = loopMap.at(L);
                loopsToProcess.push_back({L, loopId});
            }
        }

        void DynmLoopUnrollPass::removeLoopTrace(Loop* L) {
            for (BasicBlock* BB : L->blocks()) {
                for (auto I = BB->begin(); I != BB->end();) {
                    if (auto* CI = dyn_cast<CallInst>(&*I)) {
                        if (CI->getCalledFunction() &&
                            CI->getCalledFunction()->getName() == "LoopTrace") {
                            auto Next = std::next(I);
                            CI->eraseFromParent();
                            I = Next;
                            continue;
                        }
                    }
                    ++I;
                }
            }
        }

        void DynmLoopUnrollPass::cleanupTraceCalls(Function& F) {
            for (auto& BB : F) {
                for (auto I = BB.begin(); I != BB.end();) {
                    if (auto* CI = dyn_cast<CallInst>(&*I)) {
                        if (CI->getCalledFunction()) {
                            StringRef Name = CI->getCalledFunction()->getName();
                            if (Name == "LoopTraceInitialization" || 
                                Name == "LoopTraceDestroy") {
                                auto Next = std::next(I);
                                CI->eraseFromParent();
                                I = Next;
                                continue;
                            }
                        }
                    }
                    ++I;
                }
            }
        }

        void DynmLoopUnrollPass::getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
            AU.setPreservesCFG();
        }

    char DynmLoopUnrollPass::ID = 1;
    static RegisterPass<DynmLoopUnrollPass> Z("DynmLoopUnroll", "DynmLoopUnroll", true, false);
} // namespace DashTracer::Passes