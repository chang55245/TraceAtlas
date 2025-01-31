#if __cplusplus < 201703L
#error "This file requires C++17 or later"
#endif

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/DataLayout.h"
#include <fstream>
#include "llvm/IR/Module.h" 
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <set>
#include <llvm/Transforms/Utils/CodeExtractor.h>
#include "llvm/Support/CommandLine.h"

using namespace llvm;

// Command line option
static cl::opt<std::string> TaskMergingInfo("tm", 
    cl::desc("Schedule information file"), 
    cl::value_desc("Schedule information file filename"), 
    cl::Required);

namespace {
std::map<int, std::set<int>> kernelControlMap;


// New Module Pass for BB_ID_Dump analysis
struct BBIDAnalysis : public AnalysisInfoMixin<BBIDAnalysis> {
    struct Result {
        std::map<int, BasicBlock*> BBIDMap;
    };
    Result run(Module &M, ModuleAnalysisManager &AM) {
        std::map<int, BasicBlock*> BBIDMap;
        
        for (auto &F : M) {
            for (auto &BB : F) {
                for (auto &inst : BB) {
                    if (auto *CI = dyn_cast<CallInst>(&inst)) {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc && calledFunc->getName() == "BB_ID_Dump") {
                            auto *cons = dyn_cast<ConstantInt>(CI->getArgOperand(0));
                            if (cons) {
                                auto number = cons->getSExtValue();
                                BBIDMap[int(number)] = &BB;
                                break;
                            }
                        }
                    }
                }
            }
        }
        return Result{BBIDMap};
    }

    static AnalysisKey Key;
};

AnalysisKey BBIDAnalysis::Key;

// Original Function Pass (simplified)
struct MergeTaskExtraction : public PassInfoMixin<MergeTaskExtraction> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {

        Module *M = F.getParent();
        ModuleAnalysisManager MAM;
        auto &result = MAM.getResult<BBIDAnalysis>(*M);
        auto &BBIDMap = result.BBIDMap;

        // extract the basic blocks of tasks to form functions
        for (auto& [key, value] : kernelControlMap) {
            errs() << "Kernel ID: " << key << " Control: ";
            for (int control : value) {
                errs() << control << " ";
            }
        }

        return PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};

// Modified Pass Plugin Registration
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "MergeTaskExtraction", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "MergeTaskExtraction") {
                        // Read the JSON file
                        nlohmann::json j;
                        std::ifstream inputStream(TaskMergingInfo);
                        inputStream >> j;
                        inputStream.close();
                        
                        for (auto& [key, value] : j["kernelControlMap"].items()) {
                            int kernelId = value[0].get<int>();
                            for (auto& control : value[1]) {
                                kernelControlMap[kernelId].insert(control.get<int>());
                            }
                        }                        
                        FPM.addPass(MergeTaskExtraction());
                        return true;
                    }
                    return false;
                });
            PB.registerAnalysisRegistrationCallback([](ModuleAnalysisManager &MAM) {
                    MAM.registerPass([] { return BBIDAnalysis(); });
                });
        }
    };
}




} // end anonymous namespace