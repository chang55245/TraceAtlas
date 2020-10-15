#include "Passes/SplitKernExitEnter.h"
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace DashTracer::Passes {

    bool SplitKernExitEnter::runOnFunction(Function &F) {
        Instruction *exitCall, *entranceCall;
        bool isModified = false;

        for (auto BB = F.begin(); BB != F.end();)
        {
            exitCall = nullptr;
            entranceCall = nullptr;

            //outs() << "Processing a new basic block\n";
            for (auto ii = BB->begin(); ii != BB->end(); ii++) {
                Instruction *inst = &*ii;
                if (auto *CI = dyn_cast<CallInst>(inst)) {
                    if (CI->getCalledFunction()->getName() == "KernelExit") {
                        //outs() << "Found a Kernel Exit\n";
                        exitCall = CI;
                    }
                    if (exitCall != nullptr && CI->getCalledFunction()->getName() == "KernelEnter") {
                        //outs() << "Found a Kernel Entrance\n";
                        entranceCall = CI;
                    }
                }
                if (exitCall != nullptr && entranceCall != nullptr) {
                    //outs() << "Splitting a basic block!\n";
                    BB->splitBasicBlock(ii);
                    isModified = true;
                    break;
                }
            }
            if (exitCall == nullptr || entranceCall == nullptr) {
                BB++;
            }
        }

        return isModified;
    }
    char SplitKernExitEnter::ID = 0;
    static RegisterPass<SplitKernExitEnter> Z("SplitKernExitEnter", "Looks for BBs containing both a KernExit and KernEnter and splits them into separate blocks", false, false);
} // namespace DashTracer::Passes