#include "Passes/DynmLoopUnroll.h"
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
                        CI->eraseFromParent();
                        return true;
                    }
                }
            }
            return false;
        }

        // Process a single loop
        bool DynmLoopUnrollPass::processLoop(Loop* L, uint64_t loopId) {
            if (loopIteration.find(loopId) == loopIteration.end()) {
                return false;
            }

            ULO.Count = loopIteration[loopId].count;
            ULO.TripCount = ULO.Count + 1;

            if (!modifyLoopBound(L, ULO.Count)) {
                return false;
            }

            bool PreserveLCSSA = L->isRecursivelyLCSSAForm(*DT, *LI);
            UnrollLoop(L, ULO, LI, SE, DT, AC, ORE, PreserveLCSSA, nullptr);
            simplifyLoopAfterUnroll(L, true, LI, SE, DT, AC);

            // Remove LoopTrace calls for this loop
            for (auto& BB : L->getBlocks()) {
                for (auto I = BB->begin(); I != BB->end();) {
                    if (auto* CI = dyn_cast<CallInst>(&*I)) {
                        if (CI->getCalledFunction() &&
                            CI->getCalledFunction()->getName() == "LoopTrace") {
                            I = CI->eraseFromParent();
                            continue;
                        }
                    }
                    ++I;
                }
            }

            return true;
        }

        // Recursively process nested loops
        bool DynmLoopUnrollPass::processNestedLoops(Loop* L, uint64_t loopId, map<Loop*, uint64_t>& loopMap) {
            bool modified = false;

            // Process child loops first
            for (Loop* SubL : L->getSubLoops()) {
                if (loopMap.count(SubL)) {
                    uint64_t subLoopId = loopMap[SubL];
                    if (loopIteration[subLoopId].parentId == loopId) {
                        modified |= processNestedLoops(SubL, subLoopId, loopMap);
                    }
                }
            }

            // Then process this loop
            modified |= processLoop(L, loopId);
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

            // Start with top-level loops
            for (Loop* L : *LI) {
                if (loopMap.count(L)) {
                    uint64_t loopId = loopMap[L];
                    if (loopIteration[loopId].parentId == 0) {
                        modified |= processNestedLoops(L, loopId, loopMap);
                    }
                }
            }

            // Clean up any remaining LoopTrace-related calls
            for (auto& BB : F) {
                for (auto I = BB.begin(); I != BB.end();) {
                    if (auto* CI = dyn_cast<CallInst>(&*I)) {
                        if (CI->getCalledFunction()) {
                            StringRef Name = CI->getCalledFunction()->getName();
                            if (Name == "LoopTraceInitialization" || 
                                Name == "LoopTraceDestroy") {
                                I = CI->eraseFromParent();
                                modified = true;
                                continue;
                            }
                        }
                    }
                    ++I;
                }
            }

            cleanupAnalysis();
            return modified;
        }

        void DynmLoopUnrollPass::getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
            AU.setPreservesCFG();
        }

    char DynmLoopUnrollPass::ID = 1;
    static RegisterPass<DynmLoopUnrollPass> Z("DynmLoopUnroll", "DynmLoopUnroll", true, false);
} // namespace DashTracer::Passes