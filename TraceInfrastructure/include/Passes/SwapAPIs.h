#pragma once
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>

using namespace llvm;

namespace DashTracer
{
    namespace Passes
    {

        struct SwapAPIs : public FunctionPass
        {
            static char ID;
            SwapAPIs() : FunctionPass(ID) { kern_instance_ctr = 0; }
            bool runOnFunction(Function &F) override;
            void getAnalysisUsage(AnalysisUsage &AU) const override;
            bool doInitialization(Module &M) override;

        private:
            uint64_t kern_instance_ctr;
        };

    } // namespace Passes
} // namespace DashTracer
