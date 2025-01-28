#include "Passes/TaskMergingReorder.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"
#include <fstream>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{

    vector<pair<int,int>> pair_of_start_end_node_in_order;
    // source node end bb -> old target start bb, new target start bb
    static map<int64_t,pair<int64_t,int64_t>> MergingBBMapingTransform;
    
   

    bool TaskMergingReorder::runOnFunction(Function &F)
    {
        map<int64_t, BasicBlock *> &BBidToPtr = getAnalysis<DashTracer::Passes::EncodedAnnotate>().getIDmap();
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
        {
            auto *block = cast<BasicBlock>(BB);
            auto dl = block->getModule()->getDataLayout();
            int64_t blockId = GetBlockID(block);
            if (MergingBBMapingTransform.find(blockId)!=MergingBBMapingTransform.end())
            {
                for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
                {
                    auto *CI = dyn_cast<Instruction>(BI);
                    if (auto *branch = dyn_cast<BranchInst>(CI))
                    {
                        // errs()<<*branch<<"\n";
                        int64_t num = branch->getNumSuccessors();
                        for (unsigned int i = 0;i< num;i++)
                        {
                            auto succ = branch->getSuccessor(i);
                            int64_t succId = GetBlockID(succ);
                            if(succId==MergingBBMapingTransform[blockId].first)
                            {
                                // change successor into the second
                                auto newBB = BBidToPtr[MergingBBMapingTransform[blockId].second];
                            
                                branch->setSuccessor(i,newBB);                      
                                // bool verifyRes = verifyFunction(F);
                                // errs() << "verfiy:" <<verifyRes<<"\n";
                            }
                        }
                        
                    }
                }
            }
            else
            {
                errs()<<"no node io found for block:"<<blockId<<"\n";
            }
        }
        return true;
    }

    bool TaskMergingReorder::doInitialization(Module &M)
    {  

        pair_of_start_end_node_in_order.clear();
        MergingBBMapingTransform.clear();
        nlohmann::json j;
        std::ifstream inputStream(TaskMergingSchedule);
        inputStream >> j;
        inputStream.close();
        
        pair_of_start_end_node_in_order = j["pair_of_start_end_node_in_order"].get<vector<pair<int,int>>>();
        

        for (auto& [key, value] : j["MergingBBMapingTransform"].items()) {
            int64_t sourceNode = std::stoll(key);  // Convert string key to int64_t
            if (value.is_array() && value.size() == 2) {
                int64_t firstValue = value[1][0].get<int64_t>();
                int64_t secondValue = value[1][1].get<int64_t>();
                
                MergingBBMapingTransform[sourceNode] = std::make_pair(firstValue, secondValue);
            } else {
                errs() << "Invalid format for MergingBBMapingTransform entry: " << key << "\n";
                return false;
            }
        }
        
        return false;
    }

    void TaskMergingReorder::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char TaskMergingReorder::ID = 1;
    static RegisterPass<TaskMergingReorder> Y("TaskMergingReorder", "TaskMergingReorder pass", true, false);
} // namespace DashTracer::Passes
