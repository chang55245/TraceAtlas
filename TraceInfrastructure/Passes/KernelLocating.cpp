#include "Passes/KernelLocating.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DataLayout.h"
#include <cstdint>
#include <fstream>
#include <llvm/IR/BasicBlock.h>
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
    set<Loop*> seenLoops;
    bool KernelLocating::runOnFunction(Function &F)
    {
        DominatorTree DT(F);
        LoopInfo LI(DT);

        if (LI.empty())
        {
            return false;
        }

        
        // for (auto fi = F.begin(); fi != F.end(); fi++)
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
                }
                else if (auto retInst = dyn_cast<ReturnInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                }
                else if (auto resumeInst = dyn_cast<ResumeInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                }
                else if (auto unreachableInst = dyn_cast<UnreachableInst>(BB->getTerminator()))
                {
                    auto endInsertion = BB->getTerminator();
                    auto *lastInst = cast<Instruction>(endInsertion);
                    IRBuilder<> lastBuilder(lastInst);
                    lastBuilder.CreateCall(LoopTraceDestroy);
                }
            }

            for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
            {
                if (auto *CI = dyn_cast<CallInst>(BI))
                {
                    Function *calledFunc = CI->getCalledFunction();
                   
                    if (calledFunc == nullptr ||!calledFunc->hasName())
                    {
                        continue;
                    }
                    if (calledFunc->getName() == "DASH_FFT"||calledFunc->getName() == "DASH_GEMM")
                    {
                        auto *outterLoop = LI.getLoopFor(dyn_cast<BasicBlock>(BB));
                        if (outterLoop != nullptr && seenLoops.find(outterLoop)==seenLoops.end())
                        {
                            seenLoops.insert(outterLoop);
                            uint64_t loopID = seenLoops.size();

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

    bool KernelLocating::doInitialization(Module &M)
    {

        LoopTrace = cast<Function>(M.getOrInsertFunction("LoopTrace", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        LoopTraceInitialization = cast<Function>(M.getOrInsertFunction("LoopTraceInitialization", Type::getVoidTy(M.getContext())).getCallee());
        LoopTraceDestroy = cast<Function>(M.getOrInsertFunction("LoopTraceDestroy", Type::getVoidTy(M.getContext())).getCallee());
        return false;
    }

    void KernelLocating::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char KernelLocating::ID = 1;
    static RegisterPass<KernelLocating> Y("KernelLocating", "locate the kernel related code", true, false);
} // namespace DashTracer::Passes