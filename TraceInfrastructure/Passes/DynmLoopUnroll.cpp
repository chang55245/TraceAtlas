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
#include <system_error>



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
    map<uint64_t, uint64_t> loopIteration;
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
        ULO.PeelCount = ULO.Count;
        ULO.UnrollRemainder = true;
        ULO.ForgetAllSCEV = true;

        // LI.Blocks

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
                        if (calledFunc->getName() == "LoopTrace")
                        {
                            auto *loopTest = LI.getLoopFor(&bb);
                            if (loopTest != nullptr)
                            {
                                auto arg = CI->arg_begin();
                                const Use *use = arg;
                                Value *val = use->get();
                                ConstantInt *cons = dyn_cast<ConstantInt>(val);

                                if (loopIteration[cons->getSExtValue()]==0) {
                                    goto endloop;
                                }

                                ULO.Count = loopIteration[cons->getSExtValue()];
                                ULO.TripCount = ULO.Count+1;
                                // delete loopTrace
                                CI->eraseFromParent();

                                bool PreserveLCSSA = loopTest->isRecursivelyLCSSAForm(DT, LI);
                                // errs()<<*loopTest;


                                Value *IndVar;
                                
                               
                                BasicBlock *Header = loopTest->getHeader();
                                Instruction *Term = Header->getTerminator();
                                
                                for (BasicBlock::iterator BIi = Header->begin(), BEi = Header->end(); BIi != BEi; ++BIi)
                                {
                                    if (auto *CIi = dyn_cast<CmpInst>(BIi)) {
                                        auto cmpVar = CIi->getOperand(0);
                                        errs()<<"Found cmpVar: "<<*cmpVar<<"\n";
                                        errs()<<"Found CI: "<<*CIi<<"\n";
                                        errs()<<"Found terminaler: "<<*(Term)<<"\n";
                                        if (CIi == Term->getOperand(0)) {
                                            

                                            IndVar = cmpVar;
                                            
                                            Value *NewTripCount = ConstantInt::get(IndVar->getType(), ULO.Count);

                                            IRBuilder<> Builder(Header);

                                            Builder.SetInsertPoint(CIi);
                                            
                                            Value *NewCond = Builder.CreateICmpSLT(IndVar, NewTripCount);

                                            
                                            
                                            BranchInst *NewTerm = BranchInst::Create(Term->getSuccessor(0),
                                                                                Term->getSuccessor(1), NewCond);
                                            ReplaceInstWithInst(Term, NewTerm);
                                                         
                                            CIi->eraseFromParent();
                                            // errs()<<"Found NewCond: "<<*NewCond<<"\n";
                                            // errs()<<"Found NewTerm: "<<*NewTerm<<"\n";             
                                            break;
                                        }
                                    }
                                }           
                                

                                LI.verify(DT);                                
                                UnrollLoop(loopTest, ULO, &LI, &SE, &DT, &AC, &ORE, PreserveLCSSA, nullptr);
                                simplifyLoopAfterUnroll(loopTest, true,&LI, &SE, &DT, &AC);
                                unrolled = true;
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
        mapping = j["loopIteration"];
        loopIteration = mapping.get<map<uint64_t, uint64_t>>();
        return false;
    }

    void DynmLoopUnroll::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char DynmLoopUnroll::ID = 1;
    static RegisterPass<DynmLoopUnroll> Z("DynmLoopUnroll",
                                          "DynmLoopUnroll",
                                          true, false);
} // namespace DashTracer::Passes