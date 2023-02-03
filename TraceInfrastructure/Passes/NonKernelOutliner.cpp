#include "Passes/NonKernelOutliner.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <llvm-9/llvm/IR/BasicBlock.h>
#include <llvm-9/llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"



using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
   
    using nodeInfo = struct nodeInfo
    {
        string label;
        vector <int64_t> bbs;
    };
    map<string,nodeInfo> NodeBB;
    map<int,set<int>> nonkernelStage;
    map<int64_t, BasicBlock *> BBidToPtr;

    void from_json(const nlohmann::json& j, nodeInfo& p) 
    {
        j.at("Blocks").get_to(p.bbs);
        j.at("Label").get_to(p.label);
    }

    


    void Outline(vector<int64_t> BBs)
    {
        vector<BasicBlock*> candidates;
        candidates.reserve(BBs.size());
        for (auto blockID: BBs) 
        {
            candidates.push_back(BBidToPtr[blockID]);
        }
        CodeExtractor CE(candidates);
        Function *Outlined = CE.extractCodeRegion();
        errs()<<" ce eligible: " << CE.isEligible() << ", outlined function:" << Outlined<<"\n";
    }
    
    bool NonKernelOutliner::runOnFunction(Function &F)
    {
        BBidToPtr = getAnalysis<DashTracer::Passes::EncodedAnnotate>().getIDmap();
        uint64_t max_stage_size = 0;
        
        if (F.getName() != "main")
        {
            return false;
        }

        for (auto stage : nonkernelStage)
        {
            
            if (stage.second.size()>1) 
            {
                uint64_t stage_size  = stage.second.size();
                if (stage_size > max_stage_size) {
                    max_stage_size = stage_size;
                }

                //0. get the start and end of stage, represented by non kernel id 
                int stage_start = (*stage.second.begin());
                // errs()<< "stage start "<<stage_start<<"\n";
                int stage_end = (*stage.second.rbegin());
                // errs()<< "stage end "<<stage_end<<"\n";
                
                // skip the parallelization for the failed extraction case
                bool skip = false;



                // 1. extract the code for non kernel
                for (auto nk : stage.second) 
                {
                    // errs()<< "in stage:  "<<nk<<"\n";

                    vector<int64_t> BBs = NodeBB[to_string(nk)].bbs;
                    vector<BasicBlock*> candidates;
                    for (auto blockID: BBs) 
                    {
                        BasicBlock* block = BBidToPtr[blockID];
                        if(block->getParent()->getName() =="main")
                        {
                            candidates.push_back(block);
                        }
                    }
                    // errs()<<"candidate size:" << candidates.size()<<"\n";
         
                    // aggregate for pthread 
                    CodeExtractor CE(candidates,NULL,true,NULL, NULL,NULL, false,false);
                    // CodeExtractor CE(candidates);
                    Function *Outlined = CE.extractCodeRegion();    
                    

                    if (Outlined == nullptr) {
                        skip = true;
                        break;
                        //errs()<<" ce eligible: " << CE.isEligible()<< ", node: "<<nk <<"\n";
                    }
                     
                    // errs()<<" ce eligible: " << CE.isEligible()<< ", node: "<<nk <<", outlined function:" << Outlined->getName()<<"\n";
                    string name = "nk_" + to_string(nk);
                    Outlined->setName(name);
                    errs()<<" ce eligible: " << static_cast<int>(CE.isEligible())<< ", node: "<<nk <<", outlined function:" << Outlined->getName()<<"\n";
                   
                }

                if (!skip) // succeffully extracted, should parallel
                {
                    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
                    {
                        std::vector<Value *> args;
                        Value *StageValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), stage_size);
                        args.push_back(StageValue);

                        for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
                        {
                            if (auto *CI = dyn_cast<CallInst>(BI))
                            {
                                Function *calledFunc = CI->getCalledFunction();
                                // add to each non kernel stage start, stage size
                                if (calledFunc->getName() == "nk_"+ to_string(stage_start))
                                {
                                    IRBuilder<> Builder(CI);
                                    Builder.CreateCall(StartNonKernelStage, args);
                                    break;
                                }
                                // add to each non kernel stage end, stage size
                                if (calledFunc->getName() == "nk_"+ to_string(stage_end))
                                {
                                    IRBuilder<> Builder(CI->getNextNode());
                                    Builder.CreateCall(EndNonKernelStage, args);
                                    break;
                                } 
                            }
                        }
                    } 
                }
            }
        }

        // add to main start: size of thread id array
        
        auto *first_in_main = dyn_cast<Instruction>(F.begin()->begin());
        std::vector<Value *> args;
        Value *StageValue = ConstantInt::get(Type::getInt64Ty(F.begin()->getContext()), max_stage_size);
        args.push_back(StageValue);
        IRBuilder<> Builder(first_in_main);
        Builder.CreateCall(MainStart, args); 

        return true;
    }

    bool NonKernelOutliner::doInitialization(Module &M)
    {
        
        nlohmann::json j;
        std::ifstream inputStream(NodeBBs);
        inputStream >> j;
        inputStream.close();
        nlohmann::json mapping;
        mapping = j["Control"];
        // int64_t counter = 0;
        // for(auto it = mapping.begin(); it != mapping.end(); ++it)
        // {
        //     NodeBB[counter] = mapping.at(to_string( counter)).get<nodeInfo>();
        //     counter++;
        // }
        NodeBB = mapping.get<map<string,nodeInfo>>();


        nlohmann::json k;
        std::ifstream inputStreamDAG(DAGBBIOFile);
        inputStreamDAG >> k;
        inputStreamDAG.close();
        nlohmann::json DAGmapping;
        DAGmapping = k["nonkernelStage"];
        nonkernelStage = DAGmapping.get<map<int,set<int>>>();


        MainStart = cast<Function>(M.getOrInsertFunction("MainStart", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        StartNonKernelStage = cast<Function>(M.getOrInsertFunction("StartNonKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        EndNonKernelStage = cast<Function>(M.getOrInsertFunction("EndNonKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());
        // the stage info functions, I don't need to build them at the back end, since they are all just dummy calls, will opt can't compile this?

        return false;
    }

    void NonKernelOutliner::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char NonKernelOutliner::ID = 1;
    static RegisterPass<NonKernelOutliner> Y("NonKernelOutliner", "generate test tranformation", true, false);
} // namespace DashTracer::Passes