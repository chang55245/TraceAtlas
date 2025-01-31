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
#include "llvm/Support/CommandLine.h"
using namespace llvm;




struct MergeTaskExtraction : public FunctionPass {
  static char ID;
  MergeTaskExtraction() : FunctionPass(ID) { kern_instance_ctr = 0; }
  auto runOnFunction(Function &F) -> bool override;
  auto doInitialization(Module &M) -> bool override;

private:
  uint64_t kern_instance_ctr;
};


std::map<int, std::set<int>> kernelControlMap;
cl::opt<std::string> TaskMergingInfo("tm", cl::desc("Schedule information file"), cl::value_desc("Schedule information file filename"), cl::Required);

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
    
    return false;
}

 // namespace DashTracer::Passes

char MergeTaskExtraction::ID = 1;
static RegisterPass<MergeTaskExtraction> Y("MergeTaskExtraction", "MergeTaskExtraction pass", true, false);
