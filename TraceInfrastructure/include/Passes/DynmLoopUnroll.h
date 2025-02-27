#pragma once
#include <llvm/IR/BasicBlock.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/AssumptionCache.h>
#include <llvm/Analysis/OptimizationRemarkEmitter.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <map>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>




#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
using namespace llvm;

#pragma clang diagnostic ignored "-Woverloaded-virtual"

namespace DashTracer::Passes {
    struct LoopProfileInfo {
        uint64_t count;
        uint64_t parentId;
    };

    class DynmLoopUnrollPass : public FunctionPass
    {
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

        void initializeUnrollOptions();
        void initializeAnalysis(Function &F);
        void cleanupAnalysis();
        std::map<Loop*, uint64_t> findLoopsWithTraces(Function &F);
        bool modifyLoopBound(Loop* L, uint64_t count);
        bool processLoop(Loop* L, uint64_t loopId);
        bool processNestedLoops(Loop* L, uint64_t loopId, std::map<Loop*, uint64_t>& loopMap);

    public:
        static char ID;
        DynmLoopUnrollPass() : FunctionPass(ID) {}
        bool runOnFunction(Function &F) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool doInitialization(Module &M) override;
    };
} // namespace DashTracer::Passes