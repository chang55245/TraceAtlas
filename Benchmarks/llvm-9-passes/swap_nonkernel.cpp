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

#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <llvm-9/llvm/Support/raw_ostream.h>

using namespace llvm;
using namespace std;

struct SwapNonkernel : public FunctionPass {
  static char ID;
  SwapNonkernel() : FunctionPass(ID) { kern_instance_ctr = 0; }
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;

private:
  uint64_t kern_instance_ctr;
};

bool SwapNonkernel::runOnFunction(Function &F) {
  bool modified = false;

  if (F.getName() != "main") {
    return modified;
  }

  Module *module = F.getParent();

  // For use in debugging through CLion by whitelisting CLion's PID as something
  // that can trace this process prctl(PR_SET_PTRACER, 17808, 0, 0, 0);

  Function *pthread_create = module->getFunction("pthread_create");
  if (!pthread_create) {
    errs() << "Unable to find the pthread_create function!\n";
    return modified;
  }

  Function *pthread_join = module->getFunction("pthread_join");
  if (!pthread_join) {
    errs() << "Unable to find the pthread_join function!\n";
    return modified;
  }

 
  errs()<< "pass";
  modified = false;
  return modified;
}

bool SwapNonkernel::doInitialization(Module &M) {
  bool modified = false;
  for (Module::iterator F = M.begin(), E = M.end(); F != E;) {
    Function *func = &*F;
    F++;
    if (func->getName() == "dummy_function") {
      func->eraseFromParent();
      modified = true;
    }
  }

  return modified;
}

char SwapNonkernel::ID = 0;
static RegisterPass<SwapNonkernel> Z("SwapNonkernel",
                                "Find instances of DASH API calls throughout a given set of LLVM modules "
                                "and swap them for CEDR invocations",
                                false, false);
