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

struct erase : public FunctionPass {
  static char ID;
  erase() : FunctionPass(ID) { kern_instance_ctr = 0; }
  auto runOnFunction(Function &F) -> bool override;
  auto doInitialization(Module &M) -> bool override;

private:
  uint64_t kern_instance_ctr;
};

auto erase::runOnFunction(Function &F) -> bool {

  bool modified = false;


        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;) {
    Instruction *inst = &*I;
    I++;

    if (auto *CI = dyn_cast<CallInst>(inst)) {
      Function *calledFunc = CI->getCalledFunction();

      if (calledFunc) {
        auto funcName = calledFunc->getName();
        // name start mid
        if (funcName == "LoadDump"|| funcName == "StoreDump"|| funcName == "MemCpyDump"|| funcName == "BB_ID_Dump") {
          inst->eraseFromParent();
        }
      }
    }
  }


    // Return true to indicate that the function was modified
    return true;
  
}

  auto erase::doInitialization(Module & M)->bool {
    bool modified = false;
    return modified;
  }

  char erase::ID = 0;
  static RegisterPass<erase> X("erase", "My Pass", false, false);