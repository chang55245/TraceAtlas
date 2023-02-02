#include "Passes/NonKernelOutliner.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"
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

        // for (auto i:nonkernelStage[11]) {
        //     errs() << i;
        // }
        
        if (F.getName() != "main")
        {
            return false;
        }
        for (const auto& node: NodeBB) 
        {
            // this is a non kernel
            if (node.second.label=="-1") 
            {
                vector<int64_t> BBs = node.second.bbs;
                vector<BasicBlock*> candidates;
                for (auto blockID: BBs) 
                {
                    BasicBlock* block = BBidToPtr[blockID];
                    if(block->getParent()->getName() =="main")
                    {
                        candidates.push_back(block);
                    }
                }
                errs()<<"candidate size:" << candidates.size()<<"\n";
                
                // aggregate for pthread 
                CodeExtractor CE(candidates,NULL,true,NULL, NULL,NULL, true,true);
                // CodeExtractor CE(candidates);
                Function *Outlined = CE.extractCodeRegion();
                errs()<<" ce eligible: " << CE.isEligible()<< ", node: "<<node.first <<", outlined function:" << Outlined<<"\n";
            }      
        }    
    
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
        // int64_t counter = 0;
        // for(auto it = mapping.begin(); it != mapping.end(); ++it)
        // {
        //     NodeBB[counter] = mapping.at(to_string( counter)).get<nodeInfo>();
        //     counter++;
        // }
        nonkernelStage = DAGmapping.get<map<int,set<int>>>();
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