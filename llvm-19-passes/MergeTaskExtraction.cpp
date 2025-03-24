#include "llvm/ADT/SetVector.h"
#if __cplusplus < 201703L
#error "This file requires C++17 or later"
#endif
#include "MergeTaskExtraction.h"
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
#include <llvm/IR/InstIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <set>
#include <llvm/Transforms/Utils/CodeExtractor.h>
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

using namespace llvm;

// Command line option
static cl::opt<std::string> TaskMergingSchedule("tm", 
    cl::desc("Schedule information file"), 
    cl::value_desc("Schedule information file filename"), 
    cl::Optional);


namespace {


// merged task id --> basic block ids
std::map<int, std::set<int>> taskMergingNodeMap;
std::map<int, std::set<BasicBlock*>> taskMergingBB;
std::map<int, int> merged_node_start_bb;
// New Module Pass for BB_ID_Dump analysis
struct BBIDAnalysis : public AnalysisInfoMixin<BBIDAnalysis> {
    using Result = BBIDAnalysisInfo;
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
    void whyBBNotWorking(std::vector<BasicBlock*> &BlocksToExtract) {
        bool AllowVarArgs = true, AllowAlloca = true;
        SetVector<BasicBlock *> BBSet(BlocksToExtract.begin(), BlocksToExtract.end());

        for (BasicBlock *BB : BlocksToExtract) {
            
            // Check if block address is taken
            if (BB->hasAddressTaken()) {
                errs() << "  - Block address is taken (BlockAddress used).\n";
                continue;
            }

            // Check if the block or its instructions reference a BlockAddress
            for (Instruction &I : *BB) {
                for (User *U : I.users()) {
                    if (isa<BlockAddress>(U)) {
                        errs() << "  - Block references a BlockAddress.\n";
                        goto NextBlock;
                    }
                }

                // Check for Alloca usage
                if (isa<AllocaInst>(I) && !AllowAlloca) {
                    errs() << "  - Contains alloca instruction, but AllowAlloca is false.\n";
                }

                // Check for invoke unwind destination validity
                if (auto *II = dyn_cast<InvokeInst>(&I)) {
                    if (BasicBlock *UBB = II->getUnwindDest()) {
                        if (!BBSet.count(UBB)) {
                            errs() << "  - Invoke instruction unwinds to a block outside extraction set.\n";
                        }
                    }
                }

                // Check for catchswitch instruction validity
                if (auto *CSI = dyn_cast<CatchSwitchInst>(&I)) {
                    if (BasicBlock *UBB = CSI->getUnwindDest()) {
                        if (!BBSet.count(UBB)) {
                            errs() << "  - CatchSwitch unwinds to a block outside extraction set.\n";
                        }
                    }
                    for (BasicBlock *HBB : CSI->handlers()) {
                        if (!BBSet.count(HBB)) {
                            errs() << "  - CatchSwitch handler is outside the extraction set.\n";
                        }
                    }
                }

                // Check for cleanup handlers
                if (auto *CPI = dyn_cast<CleanupPadInst>(&I)) {
                    for (User *U : CPI->users()) {
                        if (auto *CRI = dyn_cast<CleanupReturnInst>(U)) {
                            if (!BBSet.count(CRI->getParent())) {
                                errs() << "  - CleanupPadInst has an exit block outside extraction set.\n";
                            }
                        }
                    }
                }

                // Check for calls to problematic intrinsics
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (const Function *F = CI->getCalledFunction()) {
                        auto IID = F->getIntrinsicID();
                        if (IID == Intrinsic::vastart && !AllowVarArgs) {
                            errs() << "  - Calls vastart but AllowVarArgs is false.\n";
                        }
                        if (IID == Intrinsic::eh_typeid_for) {
                            errs() << "  - Calls eh_typeid_for, which is not extractable.\n";
                        }
                    }
                }
            }

            // Ensure block is not a landing pad (if it's the entry block)
            if (BB == BlocksToExtract.front() && BB->isEHPad()) {
                errs() << "  - The first block cannot be a landing pad.\n";
            }

            // Ensure blocks (except the first) don't have predecessors outside the region
            for (BasicBlock *Pred : predecessors(BB)) {
                if (!BBSet.count(Pred) && BB != BlocksToExtract.front()) {
                    errs() << "  - Block has predecessors outside extraction region.\n";
                }
            }

        NextBlock:
            continue;
        }

        bool update = true;
        while (update) {
            update = false;

            for (BasicBlock *BB : BlocksToExtract) {
                for (BasicBlock *Pred : predecessors(BB)) {
                if (!BBSet.count(Pred) && BB != BlocksToExtract.front()) {
                        errs() << "update bb extraction set.\n";
                        errs() << "Pred: " << Pred << "\n";
                        for (auto& [key, value] : taskMergingBB) {
                            if (value.count(Pred)) {
                                errs() << "Pred is in taskMergingBB\n";
                                errs() << "key: " << key << "\n";
                            }
                        }
                        
                        BlocksToExtract.push_back(Pred);
                        BBSet.insert(Pred);
                        update = true;
                        break;
                    }
                }
            }
        }
    }
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;) {
            Instruction *inst = &*I;
            I++;
            
                if (auto *CI = dyn_cast<CallInst>(inst)) {
                    Function *calledFunc = CI->getCalledFunction();
                    if (calledFunc) {
                        auto funcName = calledFunc->getName();
                        // name start mid
                        if (funcName == "LoadDump"|| funcName == "StoreDump"|| funcName == "MemCpyDump"|| funcName == "ComputeDump"|| funcName == "BB_ID_Dump") {
                            inst->eraseFromParent();
                        }
                    }
                }
            
        }

        if (F.getName() != "main") {
            return PreservedAnalyses::all();
        }

        // Get the module analysis results through the proxy
        auto &MAMProxy = FAM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
        
        // Get the analysis result directly through the proxy
        BBIDAnalysisInfo  *BBIDInfo = MAMProxy.getCachedResult<BBIDAnalysis>(*F.getParent());

        std::map<int, BasicBlock*> BBIDMap = BBIDInfo->BBIDMap;

        // extract the basic blocks of tasks to form functions
        for (auto& [key, value] : taskMergingNodeMap) {
            for (auto& bb : value) {
                taskMergingBB[key].insert(BBIDMap[bb]);
            }
        }

        

        for (auto& [key, value] : taskMergingNodeMap) {
            std::vector<BasicBlock*> taskBBs;

           
            for (int bb : value) {
                BasicBlock *BBPtr = BBIDMap[bb];
                if (BBPtr->getParent()->getName() == "main") {
                    
                    if (merged_node_start_bb[key] == bb) {
                        taskBBs.insert(taskBBs.begin(), BBPtr);
                    }
                    else {
                        taskBBs.push_back(BBPtr);                  
                    }
                    
                    // errs() <<   "Task BB id: " << BB << "\n" << "BB block: " << *BBPtr << "\n";
                }
            }

            whyBBNotWorking(taskBBs);

            // Function *Fptr = &F;
            // CodeExtractor CE(taskBBs, /* DominatorTree */ nullptr,
            //        /* AggregateArgs */ false, /* BlockFrequencyInfo */ nullptr,
            //        /* BranchProbabilityInfo */ nullptr,
            //        /* AssumptionCache */ nullptr,
            //        /* AllowVarArgs */ false,
            //        /* AllowAlloca */ false,
            //        /* AllocaBlock*/ nullptr,
            //        /* Suffix */ "",
            //        /* ArgsInZeroAddressSpace */ false);

            CodeExtractor CE(taskBBs);

             // isBlockValidForExtraction is the function that checks if the task is extractable
             // todo: for unextractable tasks, we might need to manually manage them in taskflow IR
            if (!CE.isEligible()) {
                errs() << "Task id: " << key << " not Extractable: " << CE.isEligible() << "\n";
                continue;
            }

            errs() << "Extracting task: " << key << " Extractable: " << CE.isEligible() << "\n";
            CodeExtractorAnalysisCache CEAC(F);
            Function *newFunc = CE.extractCodeRegion(CEAC);
            errs() << "Extracted function: " << newFunc << "\n";
        
            // set the name of the new function
            newFunc->setName("taskflow_task_" + std::to_string(key));

            // todo dont extract the first and last task, may not be necessary because of graph reordering
           
        }

        return PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};


struct MergeTaskExtractionWrapper : public PassInfoMixin<MergeTaskExtractionWrapper> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        AM.getResult<BBIDAnalysis>(M);
        FunctionPassManager FPM;
        FPM.addPass(MergeTaskExtraction());
        auto FPMAdaptor = createModuleToFunctionPassAdaptor(std::move(FPM));
        return FPMAdaptor.run(M, AM);
    }
};
struct RemoveMainPass : public PassInfoMixin<RemoveMainPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
        // Find and remove the main function
        if (Function *Main = M.getFunction("main")) {
            Main->deleteBody();
            Main->eraseFromParent();
        }
        return PreservedAnalyses::all();
    }
};

// Modified Pass Plugin Registration
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "MergeTaskExtraction", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "MergeTaskExtraction") {
                        // Read the JSON file
                        nlohmann::json j;
                        if (TaskMergingSchedule.empty()) {
                            errs() << "Task merging schedule file not provided\n";
                            return false;
                        }
                        // read the task merging node file
                        std::ifstream inputStream(TaskMergingSchedule);
                        inputStream >> j;
                        inputStream.close();

                        for (auto& [key, value] : j["merged_from_nodes"].items()) {
                            int taskId = value[0].get<int>();
                            taskMergingNodeMap[taskId] = value[1].get<std::set<int>>();
                        }
                        for (auto& [key, value] : j["merged_node_start_bb"].items()) {
                            int taskId = value[0].get<int>();
                            merged_node_start_bb[taskId] = value[1].get<int>();
                        }

                        MPM.addPass(MergeTaskExtractionWrapper());
                                                
                        return true;
                    }
                    return false;
                });
            PB.registerAnalysisRegistrationCallback([](ModuleAnalysisManager &MAM) {
                    MAM.registerPass([] { return BBIDAnalysis(); });
                });
            PB.registerPipelineParsingCallback(
            [](StringRef Name, ModulePassManager &MPM,
                ArrayRef<PassBuilder::PipelineElement>) {
                // Make sure this exactly matches what you use in the opt command
                if (Name == "remove-main") {
                    MPM.addPass(RemoveMainPass());
                    return true;
                }
                return false;
            });
        }
    };
}




} // end anonymous namespace