// MyPasses.h
#ifndef MYPASSES_H
#define MYPASSES_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include <map>
#include <set>

namespace llvm {

// Module Analysis Pass
struct BBIDAnalysisInfo {
    std::map<int, BasicBlock*> BBIDMap;
    bool invalidate(Module &M, const PreservedAnalyses &PA,
                   ModuleAnalysisManager::Invalidator &Inv) {
        // Return false if this analysis result remains valid
        // or true if it needs to be recomputed
        return false;
    }
};

class BBIDAnalysis : public AnalysisInfoMixin<BBIDAnalysis> {
    friend AnalysisInfoMixin<BBIDAnalysis>;
    static AnalysisKey Key;

public:
    using Result = BBIDAnalysisInfo;
    Result run(Module &M, ModuleAnalysisManager &MAM);
};

// Function Transformation Pass
class MergeTaskExtraction : public PassInfoMixin<MergeTaskExtraction> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};

} // namespace llvm

#endif // MYPASSES_H