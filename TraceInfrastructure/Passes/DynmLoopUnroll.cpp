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
            ULO.Force = true;
            ULO.AllowRuntime = false;
            ULO.AllowExpensiveTripCount = true;
            ULO.PreserveCondBr = true;
            ULO.PreserveOnlyFirst = false;
            ULO.TripMultiple = 0;
            ULO.PeelCount = 0;
            ULO.UnrollRemainder = true;
            ULO.ForgetAllSCEV = false;
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
            if (!L) {
                errs() << "Warning: Loop is null in verifyLoopStructure\n";
                return false;
            }

            errs() << "Verifying loop: " << L->getName() << "\n";

            if (!L->getHeader()) {
                errs() << "Warning: Loop header is null for loop: " << L->getName() << "\n";
                errs() << "Loop Blocks: \n";
                for (auto* BB : L->blocks()) {
                    errs() << BB->getName() << "\n";
                }
                return false;
            }

            if (!L->getLoopPreheader()) {
                errs() << "Warning: Loop preheader is null for loop: " << L->getName() << "\n";
                errs() << "Loop Blocks: \n";
                for (auto* BB : L->blocks()) {
                    errs() << BB->getName() << "\n";
                }
                return false;
            }

            if (!L->getLoopLatch()) {
                errs() << "Warning: Loop latch is null for loop: " << L->getName() << "\n";
                return false;
            }

            SmallVector<BasicBlock*, 4> ExitBlocks;
            L->getExitBlocks(ExitBlocks);
            if (ExitBlocks.empty()) {
                errs() << "Warning: Loop exit blocks are empty for loop: " << L->getName() << "\n";
                return false;
            }

            if (!L->isLoopSimplifyForm()) {
                errs() << "Warning: Loop is not in simplify form: " << L->getName() << "\n";
                return true;
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
            if (!Header) return false;
            
            errs() << "Modifying loop bound to " << count << "\n";
            
            BranchInst* BI = dyn_cast<BranchInst>(Header->getTerminator());
            if (!BI || !BI->isConditional()) {
                errs() << "Loop header does not end with conditional branch\n";
                return false;
            }

            CmpInst* CI = dyn_cast<CmpInst>(BI->getCondition());
            if (!CI) {
                errs() << "Loop condition is not a comparison\n";
                return false;
            }

            // Print original comparison details
            errs() << "Original comparison: " << *CI << "\n";
            errs() << "Predicate: " << CI->getPredicateName(CI->getPredicate()) << "\n";
            errs() << "LHS: " << *CI->getOperand(0) << "\n";
            errs() << "RHS: " << *CI->getOperand(1) << "\n";

            // Get the operands
            Value* IndVar = CI->getOperand(0);
            Value* OldTripCount = CI->getOperand(1);
            
            // Store the original trip count before replacing
            originalConditions[L] = OldTripCount;

            // Create new trip count constant matching the type of the induction variable
            Value* NewTripCount = ConstantInt::get(IndVar->getType(), count);

            // Only replace the trip count in the comparison instruction
            CI->setOperand(1, NewTripCount);

            // Print modified comparison details
            errs() << "Modified comparison: " << *CI << "\n";
            errs() << "New predicate: " << CI->getPredicateName(CI->getPredicate()) << "\n";
            errs() << "New LHS: " << *CI->getOperand(0) << "\n";
            errs() << "New RHS: " << *CI->getOperand(1) << "\n";

            return true;
        }

        // Process a single loop
        bool DynmLoopUnrollPass::processLoop(Loop* L, uint64_t loopId) {
            if (!L || !L->getHeader()) return false;

            Function &F = *L->getHeader()->getParent();
            uint64_t count = loopIteration[loopId].count;

            errs() << "Processing loop " << loopId << " with iteration count: " << count << "\n";

            // Initialize unroll options
            ULO.Count = count;
            ULO.TripCount = count + 1;  // Match your old implementation
            ULO.Force = false;
            ULO.AllowRuntime = false;
            ULO.AllowExpensiveTripCount = false;
            ULO.PreserveCondBr = false;
            ULO.PreserveOnlyFirst = false;
            ULO.TripMultiple = 0;
            ULO.PeelCount = count;
            ULO.UnrollRemainder = true;
            ULO.ForgetAllSCEV = true;

            // Modify loop bound using your original approach
            BasicBlock* Header = L->getHeader();
            Instruction* Term = Header->getTerminator();

            Value* IndVar = nullptr;
            for (BasicBlock::iterator BIi = Header->begin(), BEi = Header->end(); BIi != BEi; ++BIi) {
                if (auto* CIi = dyn_cast<CmpInst>(BIi)) {
                    auto cmpVar = CIi->getOperand(0);
                    errs() << "Found cmpVar: " << *cmpVar << "\n";
                    errs() << "Found CI: " << *CIi << "\n";
                    errs() << "Found terminator: " << *Term << "\n";
                    
                    if (CIi == Term->getOperand(0)) {
                        IndVar = cmpVar;
                        Value* NewTripCount = ConstantInt::get(IndVar->getType(), ULO.Count);

                        IRBuilder<> Builder(Header);
                        Builder.SetInsertPoint(CIi);
                        Value* NewCond = Builder.CreateICmpSLT(IndVar, NewTripCount);

                        BranchInst* NewTerm = BranchInst::Create(
                            Term->getSuccessor(0),
                            Term->getSuccessor(1),
                            NewCond
                        );
                        ReplaceInstWithInst(Term, NewTerm);
                        CIi->eraseFromParent();
                        break;
                    }
                }
            }

            if (!IndVar) {
                errs() << "Failed to find induction variable\n";
                return false;
            }

            // Verify and unroll
            LI->verify(*DT);
            bool PreserveLCSSA = L->isRecursivelyLCSSAForm(*DT, *LI);
            LoopUnrollResult Result = UnrollLoop(L, ULO, LI, SE, DT, AC, ORE, PreserveLCSSA);

            if (Result == LoopUnrollResult::Unmodified) {
                errs() << "Failed to unroll loop\n";
                return false;
            }

            // Simplify the loop after unrolling
            simplifyLoopAfterUnroll(L, true, LI, SE, DT, AC);
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


        bool DynmLoopUnrollPass::doInitialization(Module& /*M*/) {
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
    errs() << "Starting DynmLoopUnroll on function: " << F.getName() << "\n";
    
    // Initialize analysis
    initializeAnalysis(F);
    bool Modified = false;

    while (true) {
        bool unrolled = false;
        for (auto &BB : F) {
            for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI) {
                if (auto *CI = dyn_cast<CallInst>(BI)) {
                    Function *calledFunc = CI->getCalledFunction();
                    if (!calledFunc) continue;

                    if (calledFunc->getName() == "LoopTrace") {
                        auto *L = LI->getLoopFor(&BB);
                        if (L != nullptr) {
                            auto arg = CI->arg_begin();
                            const Use *use = arg;
                            Value *val = use->get();
                            ConstantInt *cons = dyn_cast<ConstantInt>(val);
                            uint64_t loopId = cons->getZExtValue();

                            if (loopIteration.count(loopId) == 0) {
                                goto endloop;
                            }

                            // Process the loop
                            if (processLoop(L, loopId)) {
                                Modified = true;
                                unrolled = true;
                            }
                            
                            // Remove the trace call
                            CI->eraseFromParent();
                            goto endloop;
                        }
                    }
                    else if (calledFunc->getName() == "LoopTraceInitialization") {
                        CI->eraseFromParent();
                        unrolled = true;
                        goto endloop;
                    }
                    else if (calledFunc->getName() == "LoopTraceDestroy") {
                        CI->eraseFromParent();
                        unrolled = true;
                        goto endloop;
                    }
                }
            }
        }

    endloop:
        if (!unrolled) {
            break;
        }
    }

    return Modified;
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