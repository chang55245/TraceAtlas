#include "Passes/DynmFunctionInlining.h"
#include "Passes/CommandArgs.h"
#include "Passes/Annotate.h"
#include "AtlasUtil/Annotate.h"
#include <cstdint>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Use.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>
#include <system_error>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
    map<uint64_t, uint64_t> inlineMap;
    bool DynmFunctionInlining::runOnFunction(Function &F)
    {
        return true;

        // check instructions if seen callTrace and ID is in inlineMap
        // inlineMap.find(ID) != inlineMap.end()
        // replace operand of call instruction to be dash api
        //
    }

    bool DynmFunctionInlining::doInitialization(Module &)
    {
        nlohmann::json j;
        std::ifstream inputStream(InlineMap);
        inputStream >> j;
        inputStream.close();
        nlohmann::json mapping;
        mapping = j["inlineMap"];
        inlineMap = mapping.get<map<uint64_t, uint64_t>>();
        return false;
    }

    void DynmFunctionInlining::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char DynmFunctionInlining::ID = 1;
    static RegisterPass<DynmFunctionInlining> Z("DynmFunctionInlining",
                                          "DynmFunctionInlining",
                                          true, false);
} // namespace DashTracer::Passes