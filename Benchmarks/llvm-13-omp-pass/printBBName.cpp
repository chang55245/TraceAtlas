#include "llvm/Frontend/OpenMP/OMP.h.inc"
#include "llvm/Frontend/OpenMP/OMPConstants.h"
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <nlohmann/json.hpp>
#include <forward_list>
#include <fstream>
#include <string>
#include <utility>

using namespace std;
using namespace llvm;

struct printBB : public FunctionPass {
  static char ID;
  printBB() : FunctionPass(ID) { kern_instance_ctr = 0; }
  auto runOnFunction(Function &F) -> bool override;
  auto doInitialization(Module &M) -> bool override;

private:
  uint64_t kern_instance_ctr;
};

auto printBB::runOnFunction(Function &F) -> bool {

  bool modified = false;

  Function *printf = F.getParent()->getFunction("printf");

  if (!printf) {
    errs() << "Unable to find the printf function!\n";
    return modified;
  }

  // Iterate over the basic blocks in the function
  for (BasicBlock &BB : F) {
    // Insert a new basic block at the beginning of the current block
    // Get a reference to the IR builder for the current context

    for (auto &inst : BB) {

        int BBID;
      if (auto *CI = dyn_cast<CallInst>(&inst)) {
        Function *calledFunc = CI->getCalledFunction();
        if (calledFunc->getName() == "BB_ID_Dump") {
          auto *cons = dyn_cast<ConstantInt>(CI->arg_begin()->get());
            BBID = (uint64_t)cons->getSExtValue();
        
        IRBuilder<> builder(&inst);
        // Create a constant string that specifies the format of the output
        Constant *FormatStr = builder.CreateGlobalStringPtr("BasicBlock: %s\n");

        // Create a call to the printf function
        Value *Args[] = {FormatStr, builder.CreateGlobalStringPtr(to_string(BBID))};

        // IRBuilder<>::InsertPoint StartPoint(&BB, BB.getFirstInsertionPt());

        // builder.restoreIP(StartPoint);

        builder.CreateCall(printf, Args);
        break;
        }
      }
    }
    }
    // Return true to indicate that the function was modified
    return true;
  
}

  auto printBB::doInitialization(Module & M)->bool {
    bool modified = false;
    return modified;
  }

  char printBB::ID = 0;
  static RegisterPass<printBB> X("printBB", "My Pass", false, false);