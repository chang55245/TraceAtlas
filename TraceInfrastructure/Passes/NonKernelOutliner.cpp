#include "Passes/NonKernelOutliner.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"
#include <fstream>
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

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"



using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
    using nodeInfo = struct nodeInfo
    {
        string label;
        set <int64_t> bbs;
    };

    void from_json(const nlohmann::json& j, nodeInfo& p) 
    {
        j.at("Blocks").get_to(p.bbs);
        j.at("Label").get_to(p.label);
    }

    map<int64_t,nodeInfo> NodeBB;
    bool NonKernelOutliner::runOnFunction(Function &F)
    {
        errs() << &NodeBB;
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
        {
            auto *block = cast<BasicBlock>(BB);

            // for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
            {

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
        NodeBB = mapping.get<map<int64_t,nodeInfo>>();
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