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
#include <system_error>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <functional>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
    Value *findLoadFromAlloca(Value *V, std::set<Value*> &Visited) {
  // Prevent infinite recursion
  if (Visited.count(V))
    return nullptr;
  Visited.insert(V);

  // If this is a load instruction, check if it's loading from an alloca
  if (LoadInst *Load = dyn_cast<LoadInst>(V)) {
    if (AllocaInst *Alloca = dyn_cast<AllocaInst>(Load->getPointerOperand())) {
      return Load;
    }
  }

  // Recursively check all operands
  if (Instruction *I = dyn_cast<Instruction>(V)) {
    for (Use &U : I->operands()) {
      if (Value *Result = findLoadFromAlloca(U.get(), Visited))
        return Result;
    }
  }

  return nullptr;
}

    void debugLoopUnrolling(Loop *L, UnrollLoopOptions ULO, LoopInfo &LI,
                        ScalarEvolution *SE, DominatorTree *DT,
                        AssumptionCache *AC, bool PreserveLCSSA) {
  // Check if the loop has a preheader.
  BasicBlock *Preheader = L->getLoopPreheader();
  if (!Preheader) {
    llvm::dbgs() << "Unroll failure: Loop preheader insertion failed.\n";
    return;
  }

  // Check if the loop has a latch (exit block).
  BasicBlock *LatchBlock = L->getLoopLatch();
  if (!LatchBlock) {
    llvm::dbgs() << "Unroll failure: Loop exit-block (latch) insertion failed.\n";
    return;
  }

  // Check that the loop is safe to clone.
  if (!L->isSafeToClone()) {
    llvm::dbgs() << "Unroll failure: Loop body cannot be cloned (e.g. contains indirectbr).\n";
    return;
  }

  // Retrieve the loop header and its terminator.
  BasicBlock *Header = L->getHeader();
  BranchInst *HeaderBI = dyn_cast<BranchInst>(Header->getTerminator());
  // Get the latch branch.
  BranchInst *LatchBI = dyn_cast<BranchInst>(LatchBlock->getTerminator());
  if (!LatchBI ||
      (LatchBI->isUnconditional() && (!HeaderBI || HeaderBI->isUnconditional() ||
                                      L->getExitingBlock() != Header))) {
    llvm::dbgs() << "Unroll failure: Loop is not terminated by a conditional branch "
                    "in the latch or header.\n";

    if (L->getExitingBlock() != Header) {
        errs()<<" L->getExitingBlock() != Header\n";
    }
    if (!LatchBI) {
        errs()<<" !LatchBI\n";
    }
    if (LatchBI->isUnconditional()) {
        errs()<<" LatchBI->isUnconditional()\n";
    }

    if (!HeaderBI) {
        errs()<<" !HeaderBI\n";
    }
    if (HeaderBI->isUnconditional()) {
        errs()<<" HeaderBI->isUnconditional()\n";
    }


  }

  // For loops with a conditional latch, verify that one successor is inside the loop
  // and the other is outside.
  if (LatchBI && LatchBI->isConditional()) {
    auto CheckLatchSuccessors = [LatchBI, Header, L](unsigned S1, unsigned S2) -> bool {
      return (LatchBI->getSuccessor(S1) == Header) && !L->contains(LatchBI->getSuccessor(S2));
    };
    if (!CheckLatchSuccessors(0, 1) && !CheckLatchSuccessors(1, 0)) {
      llvm::dbgs() << "Unroll failure: Conditional latch does not exit the loop properly.\n";
      return;
    }
  }

  // If the header has a conditional branch (and the latch is not conditional), check its successors.
  if (LatchBI && !LatchBI->isConditional() && HeaderBI && HeaderBI->isConditional()) {
    auto CheckHeaderSuccessors = [HeaderBI, L](unsigned S1, unsigned S2) -> bool {
      return L->contains(HeaderBI->getSuccessor(S1)) && !L->contains(HeaderBI->getSuccessor(S2));
    };
    if (!CheckHeaderSuccessors(0, 1) && !CheckHeaderSuccessors(1, 0)) {
      llvm::dbgs() << "Unroll failure: Conditional header does not exit the loop properly.\n";
      return;
    }
  }

  // Check if the loop header's address is taken.
  if (Header->hasAddressTaken()) {
    llvm::dbgs() << "Unroll failure: Loop header's address is taken.\n";
    return;
  }

  // Check if there is anything meaningful to unroll.
  if (ULO.TripCount == 0 && ULO.Count < 2 && ULO.PeelCount == 0) {
    llvm::dbgs() << "Unroll failure: Nothing to unroll (TripCount is 0 and unroll count is less than 2).\n";
    return;
  }

  // For runtime trip count loops, verify that the unroll count divides the trip multiple.
  bool RuntimeTripCount = (ULO.TripCount == 0 && ULO.Count > 0 && ULO.AllowRuntime);
  if (RuntimeTripCount && (ULO.TripMultiple % ULO.Count != 0)) {
    llvm::dbgs() << "Unroll failure: For runtime trip count loops, TripMultiple (" 
                 << ULO.TripMultiple << ") is not divisible by the unroll count (" 
                 << ULO.Count << ").\n";
    return;
  }

  llvm::dbgs() << "Loop appears to satisfy basic unrolling conditions.\n";
  // You can add additional checks or print more state information here as needed.
}


     struct LoopProfileInfo {
        uint64_t count;
        uint64_t parentId;
    };
    std::map<uint64_t, LoopProfileInfo> loopIteration;
    struct DynmLoopUnroll : public FunctionPass
        {
            static char ID;
            DynmLoopUnroll() : FunctionPass(ID) {}
            bool runOnFunction(Function &F) override;
            void getAnalysisUsage(AnalysisUsage &AU) const override;
            bool doInitialization(Module &M) override;
        
        };
    bool DynmLoopUnroll::runOnFunction(Function &F)
    {
        DominatorTree DT(F);
        LoopInfo LI(DT);
        AssumptionCache AC(F);
        TargetLibraryInfoImpl TLII;
        TargetLibraryInfo TLI(TLII);
        ScalarEvolution SE(F, TLI, AC, DT, LI);
        OptimizationRemarkEmitter ORE(&F);
        UnrollLoopOptions ULO;

        ULO.Count = 5;
        ULO.TripCount = ULO.Count;
        ULO.Force = false;
        ULO.AllowRuntime = false;
        ULO.AllowExpensiveTripCount = false;
        ULO.PreserveCondBr = false;
        ULO.PreserveOnlyFirst = false;
        ULO.TripMultiple = 0;
        ULO.PeelCount = 0;
        ULO.UnrollRemainder = true;
        ULO.ForgetAllSCEV = true;
        int64_t LoopID = 0;
        if (LI.empty())
        {
            return false; 
        }

        while (true)
        {
            bool unrolled = false;
            for (auto &bb : F)
            {
                for (BasicBlock::iterator BI = bb.begin(), BE = bb.end(); BI != BE; ++BI)
                {
                    if (auto *CI = dyn_cast<CallInst>(BI))
                    {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc == nullptr) {
                            continue;
                        }
                        if (calledFunc->getName() == "LoopTrace")
                        {
                            auto *loopTest = LI.getLoopFor(&bb);
                            if (loopTest != nullptr)
                            {
                                bool haveInnerLoopTrace = false;
                                // continue if the loop's subloop have loopTrace
                                for (auto subloop: loopTest->getSubLoops()) {
                                    for (auto BB: subloop->getBlocks()) {
                                        for (auto& Childinst: *BB) {
                                            if (auto *ChildCall = dyn_cast<CallInst>(&Childinst)) {
                                                if (ChildCall->getCalledFunction() == nullptr) {
                                                    continue;
                                                }
                                                if (ChildCall->getCalledFunction()->getName() == "LoopTrace") {
                                                    haveInnerLoopTrace = true;
                                                    break;
                                                }
                                            }
                                        }
                                        if (haveInnerLoopTrace) {
                                            break;
                                        }
                                    }
                                    if (haveInnerLoopTrace) {
                                        break;
                                    }
                                }
                                if (haveInnerLoopTrace) {
                                    continue;
                                }
                                
                                

                                auto arg = CI->arg_begin();
                                const Use *use = arg;
                                Value *val = use->get();
                                ConstantInt *cons = dyn_cast<ConstantInt>(val);

                                if (loopIteration[cons->getSExtValue()].count==0) {
                                    goto endloop;
                                }

                                ULO.Count = loopIteration[cons->getSExtValue()].count;
                                LoopID = cons->getSExtValue();
                                ULO.TripCount = ULO.Count+1;
                                // delete loopTrace
                                CI->eraseFromParent();

                                bool PreserveLCSSA = loopTest->isRecursivelyLCSSAForm(DT, LI);

                                // modify the loop bound
                                AllocaInst *InductionAlloca = nullptr;
                                Value *InductionVar = nullptr;
                                Value *BoundValue = nullptr;
                                BasicBlock *Header = loopTest->getHeader();                              
                                BasicBlock *Preheader = loopTest->getLoopPreheader();
                                BranchInst *HeaderBr = dyn_cast<BranchInst>(Header->getTerminator());
                                ICmpInst *Cmp = dyn_cast<ICmpInst>(HeaderBr->getCondition());
                                // get the init value of the loop
                                if (Cmp) {
                                    Value *Op0 = Cmp->getOperand(0);
                                    Value *Op1 = Cmp->getOperand(1);
                                    std::set<Value*> Visited;

                                    if (Value *Load = findLoadFromAlloca(Op0, Visited)) {
                                        InductionVar = Load;
                                        BoundValue = Op1;
                                        InductionAlloca = cast<AllocaInst>(cast<LoadInst>(Load)->getPointerOperand());
                                    } else {
                                        errs() << "Could not find induction variable\n";
                                    }
                                }

                                    // Get the initial value - in this case it's stored in the alloca
                                // Look for the store instruction in the entry block
                                Value *InitialValue = nullptr;
                                std::set<BasicBlock*> Visited;
                                std::function<bool(BasicBlock*)> FindInitialStore = [&](BasicBlock *BB) -> bool {
                                    // Skip if we've already visited this block
                                    if (!Visited.insert(BB).second)
                                        return false;

                                    // Search for store instruction in this block
                                    for (auto &I : *BB) {
                                        if (StoreInst *Store = dyn_cast<StoreInst>(&I)) {
                                        if (Store->getPointerOperand() == InductionAlloca) {
                                            InitialValue = Store->getValueOperand();
                                            return true;
                                        }
                                        }
                                    }

                                    // Recursively search through predecessors
                                    for (BasicBlock *Pred : predecessors(BB)) {
                                        // Only search through blocks that dominate the loop header
                                        if (DT.dominates(Pred, Header)) {
                                        if (FindInitialStore(Pred))
                                            return true;
                                        }
                                    }
                                    return false;
                                };

                                if (!FindInitialStore(Header)) {
                                    errs() << "Could not find initial value store in any dominating block\n";
                                }
                                
                                
                                // Value *BoundValue = Cmp->getOperand(1);
                                IRBuilder<> Builder(Preheader->getTerminator());
                                Type *BoundType = Cmp->getOperand(0)->getType();
                                Constant *PeelCountValue = ConstantInt::get(
                                    BoundType, ULO.Count, false);
                                Value *InitialValueCasted = Builder.CreateIntCast(InitialValue, BoundType, 
                                                     /*isSigned=*/false, "initial.casted");
                                Value *NewBound = Builder.CreateAdd(InitialValueCasted, PeelCountValue, "new_bound");
                                errs() << "NewBound: " << *NewBound << "\n";
                                errs() << "InitialValue: " << *InitialValue << "\n";
                                errs() << "Cmp: " << *Cmp << "\n";
                                ICmpInst *NewCmp = cast<ICmpInst>(Cmp->clone());
                             
                                NewCmp->setOperand(1, NewBound);
                                NewCmp->insertBefore(HeaderBr); 
                                HeaderBr->setCondition(NewCmp); 
                                Cmp->eraseFromParent();                          
                                
                                ULO.PeelCount = ULO.Count;
                                bool Result = peelLoop(loopTest, ULO.PeelCount, &LI, &SE, &DT, &AC, PreserveLCSSA);
                                if (Result == false) {
                                    errs()<<"Loop unrolling failed for loopId: "<<LoopID<<"\n";
                                }else{
                                    errs()<<"Unrolled successfully loop: "<<LoopID<<"\n";
                                }
                                simplifyLoopAfterUnroll(loopTest, true,&LI, &SE, &DT, &AC);
                                unrolled = true;

                                SE.forgetLoop(loopTest);
                                
                                goto endloop;
                                // clean the binary, delete the everything irrelavate
                            }
                        }
                        else if (calledFunc->getName() == "LoopTraceInitialization")
                        {
                            CI->eraseFromParent();
                            unrolled = true;
                            goto endloop;
                        }
                        else if (calledFunc->getName() == "LoopTraceDestroy")
                        {
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

        // errs() << F;
        return true;
    }

    bool DynmLoopUnroll::doInitialization(Module & /*unused*/)
    {
        nlohmann::json j;
        std::ifstream inputStream(LoopTraceFile);
        inputStream >> j;
        inputStream.close();
        nlohmann::json mapping;
        for (const auto& loop : j["loopIteration"]) {
                uint64_t loopId = loop["loopId"];
                loopIteration[loopId] = {
                    loop["count"],
                    loop["parentId"]
                };
            }
        return false;
    }

    void DynmLoopUnroll::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }

    char DynmLoopUnroll::ID = 1;
    static RegisterPass<DynmLoopUnroll> Z("DynmLoopUnroll",
                                          "DynmLoopUnroll");
} // namespace DashTracer::Passes