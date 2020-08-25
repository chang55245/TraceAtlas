#include "Passes/SplitAllocas.h"
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace DashTracer::Passes {

    bool SplitAllocas::runOnFunction(Function &F) {
        BasicBlock &firstBB = F.front();
        AllocaInst *furthestAlloca = nullptr;
        for (Instruction &I : firstBB) {
            if (auto *AI = dyn_cast<AllocaInst>(&I)) {
                furthestAlloca = AI;
            }
        }
        if (furthestAlloca != nullptr) {
            // We need to actually split at the immediate next instruction
            if (furthestAlloca->getNextNode() != nullptr) {
                firstBB.splitBasicBlock(furthestAlloca->getNextNode());
                return true;
            }
        }
        return false;
    }
    char SplitAllocas::ID = 0;
    static RegisterPass<SplitAllocas> Z("SplitAllocas", "Splits the first basic block of each function such that all allocas are in their own basic block", false, false);
} // namespace DashTracer::Passes