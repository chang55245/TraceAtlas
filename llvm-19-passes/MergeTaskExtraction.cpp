#if __cplusplus < 201703L
#error "This file requires C++17 or later"
#endif

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
#include <set>
#include <llvm/Transforms/Utils/CodeExtractor.h>
using namespace llvm;




struct MergeTaskExtraction : public FunctionPass {
  static char ID;
  MergeTaskExtraction() : FunctionPass(ID) { kern_instance_ctr = 0; }
  auto runOnFunction(Function &F) -> bool override;
  auto doInitialization(Module &M) -> bool override;

private:
  uint64_t kern_instance_ctr;
};
std::vector<std::pair<int,int>> pair_of_start_end_node_in_order;
// source node end bb -> old target start bb, new target start bb
std::map<int64_t,std::pair<int64_t,int64_t>> MergingBBMapingTransform;
std::map<int, std::set<int>> kernelControlMap;

    
   

auto MergeTaskExtraction::runOnFunction(Function &F) -> bool
{
  
    // extract the basic blocks of tasks to form functions
    for (auto& [key, value] : kernelControlMap) {
        // extract bb from the basic blocks of tasks
        // 1. the parent function has to be main
        // 2. the function has to be named with a dedicated name

        errs() << "Kernel ID: " << key << " Control: ";
        for (int control : value) {
            errs() << control << " ";
        }
    }

    return true;
}

bool MergeTaskExtraction::doInitialization(Module &M)
{  

    pair_of_start_end_node_in_order.clear();
    MergingBBMapingTransform.clear();
    nlohmann::json j;
    std::ifstream inputStream("TaskMergingSchedule.json");
    inputStream >> j;
    inputStream.close();
    
    pair_of_start_end_node_in_order = j["pair_of_start_end_node_in_order"].get<std::vector<std::pair<int,int>>>();
    

    for (auto& [key, value] : j["MergingBBMapingTransform"].items()) {
        if (value.is_array() && value.size() == 2) {
            int64_t firstValue = value[1][0].get<int64_t>();
            int64_t secondValue = value[1][1].get<int64_t>();
            
            MergingBBMapingTransform[value[0].get<int64_t>()] = std::make_pair(firstValue, secondValue);
        } else {
            errs() << "Invalid format for MergingBBMapingTransform entry: " << key << "\n";
            return false;
        }
    }
    
    for (auto& [key, value] : j["kernelControlMap"].items()) {
        int kernelId = value[0].get<int>();
        for (auto& control : value[1]) {
            kernelControlMap[kernelId].insert(control.get<int>());
        }
    }
    
    return false;
}

 // namespace DashTracer::Passes

char MergeTaskExtraction::ID = 1;
static RegisterPass<MergeTaskExtraction> Y("MergeTaskExtraction", "MergeTaskExtraction pass", true, false);
