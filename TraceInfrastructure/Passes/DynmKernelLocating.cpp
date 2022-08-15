#include "Passes/DynmKernelLocating.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DataLayout.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <llvm-9/llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
    //  function pass
    // find the api call and check if api call is in a loop (check the inner most loop using getloopFor)
    // if so call the loopTrace
    // for loop ID, one loop only can have one loop id, build a set: set<*loop> seenLoop
    // callinst loopTrace(int loopID)

    // Backend generate a map loopIteration<int,int>  [loopID, iterationCount]
    uint64_t currentLoopIndex = 0;
    uint64_t currentCallIndex = 0;
    bool DynmKernelLocating::runOnFunction(Function &F)
    {
        DominatorTree DT(F);
        LoopInfo LI(DT);
        for (auto BB = F.begin(); BB != F.end(); BB++)
        {
            if (F.getName() == "main")
            {
                if (BB == F.begin())
                {
                    auto firstInsertion = BB->getFirstInsertionPt();
                    auto *firstInst = cast<Instruction>(firstInsertion);
                    firstInsertion = BB->getFirstInsertionPt();
                    firstInst = cast<Instruction>(firstInsertion);
                    IRBuilder<> initBuilder(firstInst);
                    initBuilder.CreateCall(LoopTraceInitialization);
                    initBuilder.CreateCall(CallTraceInitialization);
                }
                else if (auto retInst = dyn_cast<ReturnInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                    lastBuilder.CreateCall(CallTraceDestroy);
                }
                else if (auto resumeInst = dyn_cast<ResumeInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                    lastBuilder.CreateCall(CallTraceDestroy);
                }
                else if (auto unreachableInst = dyn_cast<UnreachableInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                    lastBuilder.CreateCall(CallTraceDestroy);
                }
            }

            if (F.getName() == "DASH_FFT")
            {
                // todo: need kernel trace function, KernelTrace (kernelName)

                if (BB == F.begin())
                {
                    auto firstInsertion = BB->getFirstInsertionPt();
                    auto *firstInst = cast<Instruction>(firstInsertion);
                    firstInsertion = BB->getFirstInsertionPt();
                    firstInst = cast<Instruction>(firstInsertion);
                    IRBuilder<> initBuilder(firstInst);
                    std::vector<Value *> args;
                    Value *StageValue = ConstantInt::get(Type::getInt64Ty(dyn_cast<BasicBlock>(BB)->getContext()), (uint64_t)1);
                    args.push_back(StageValue);
                    initBuilder.CreateCall(KernelNameTrace, args);
                }    

            }


            for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
            {
                if (auto *CI = dyn_cast<CallInst>(BI))
                {
                    Function *calledFunc = CI->getCalledFunction();
                    bool checkloop = false;

                    // check if function pointer is detected
                    if (calledFunc == nullptr) {
                        currentCallIndex++;

                        //todo add a call to callTrace(callIndx)

                        uint64_t callID = currentCallIndex;
                        std::vector<Value *> args;
                        Value *StageValue = ConstantInt::get(Type::getInt64Ty(dyn_cast<BasicBlock>(BB)->getContext()), (uint64_t)callID);
                        args.push_back(StageValue);
                        IRBuilder<> Builder(CI);
                        Builder.CreateCall(CallStartTrace, args);

                        //todo check if the callEnd is in the right place, after the call instruction
                        // here next node can't be empty, since there shoule be branch at the end
                        Builder.SetInsertPoint(CI->getNextNode());
                        Builder.CreateCall(CallEndTrace, args);
                        checkloop = true;
                        errs()<<"found a function pointer \n";
                    }
                    else if (calledFunc->getName() == "DASH_FFT")
                    {
                        checkloop = true;
                    }
                                       
                    if (checkloop)
                    {
                        Loop *outterLoop = nullptr;
                        if (!LI.empty())
                        {
                            outterLoop = LI.getLoopFor(dyn_cast<BasicBlock>(BB));
                        }         
                        if (outterLoop != nullptr)
                        {
                            currentLoopIndex++;


                            uint64_t loopID = currentLoopIndex;
                            std::vector<Value *> args;
                            Value *StageValue = ConstantInt::get(Type::getInt64Ty(dyn_cast<BasicBlock>(BB)->getContext()), (uint64_t)loopID);
                            args.push_back(StageValue);
                            IRBuilder<> Builder(CI);
                            Builder.CreateCall(LoopTrace, args);
                        }
                    }
                }
            }
        }
        return true;
    }

    bool DynmKernelLocating::doInitialization(Module &M)
    {

        LoopTrace = cast<Function>(M.getOrInsertFunction("LoopTrace", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        LoopTraceInitialization = cast<Function>(M.getOrInsertFunction("LoopTraceInitialization", Type::getVoidTy(M.getContext())).getCallee());
        LoopTraceDestroy = cast<Function>(M.getOrInsertFunction("LoopTraceDestroy", Type::getVoidTy(M.getContext())).getCallee());


        CallStartTrace = cast<Function>(M.getOrInsertFunction("CallStartTrace", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        CallEndTrace = cast<Function>(M.getOrInsertFunction("CallEndTrace", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());

        CallTraceInitialization = cast<Function>(M.getOrInsertFunction("CallTraceInitialization", Type::getVoidTy(M.getContext())).getCallee());
        CallTraceDestroy = cast<Function>(M.getOrInsertFunction("CallTraceDestroy", Type::getVoidTy(M.getContext())).getCallee());

        // how llvm function call passing string in front end?
        KernelNameTrace = cast<Function>(M.getOrInsertFunction("KernelNameTrace", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        return false;
    }

    void DynmKernelLocating::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char DynmKernelLocating::ID = 1;
    static RegisterPass<DynmKernelLocating> Y("DynmKernelLocating", "locate the kernel related code", true, false);
} // namespace DashTracer::Passes