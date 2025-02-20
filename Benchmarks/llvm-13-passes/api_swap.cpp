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

using namespace llvm;
using namespace std;

struct SwapAPIs : public FunctionPass {
  static char ID;
  SwapAPIs() : FunctionPass(ID) { kern_instance_ctr = 0; }
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;

private:
  uint64_t kern_instance_ctr;
};

int CurrentKernelIndexInsideStage = 0;
map<int, AllocaInst*> cond_allocaMap;
map<int, AllocaInst*> mutex_allocaMap;
map<int, AllocaInst*> threads_doneMap;

bool function_is_swappable(std::string &funcName) {
  return funcName == "DASH_ZIP" || funcName == "DASH_FFT" || funcName == "DASH_GEMM" || funcName == "DASH_FIR" || funcName == "DASH_SpectralOpening" || funcName == "DASH_CIC";
}

bool SwapAPIs::runOnFunction(Function &F) {
  bool modified = false;
  Module *module = F.getParent();

  // For use in debugging through CLion by whitelisting CLion's PID as something
  // that can trace this process prctl(PR_SET_PTRACER, 17808, 0, 0, 0);

  Function *cedr_dispatch = module->getFunction("enqueue_kernel");
  if (!cedr_dispatch) {
    errs() << "Unable to find the cedr dispatch function!\n";
    // return modified;
  }

  Function *pthread_mutex_lock = module->getFunction("pthread_mutex_lock");
  if (!pthread_mutex_lock) {
    errs() << "Unable to find the pthread_mutex_lock function!\n";
    return modified;
  }

  Function *pthread_mutex_unlock = module->getFunction("pthread_mutex_unlock");
  if (!pthread_mutex_unlock) {
    errs() << "Unable to find the pthread_mutex_unlock function!\n";
    return modified;
  }

  Function *pthread_cond_init = module->getFunction("pthread_cond_init");
  if (!pthread_cond_init) {
    errs() << "Unable to find the pthread_cond_init function!\n";
    return modified;
  }

  Function *pthread_mutex_init = module->getFunction("pthread_mutex_init");
  if (!pthread_mutex_init) {
    errs() << "Unable to find the pthread_mutex_init function!\n";
    return modified;
  }

  Function *pthread_cond_wait = module->getFunction("pthread_cond_wait");
  if (!pthread_cond_wait) {
    errs() << "Unable to find the pthread_cond_wait function!\n";
    return modified;
  }

  // for (auto &arg : pthread_cond_init->args()) {
  //     errs()<<"arg" << *(arg.getType()->getPointerElementType())<<"\n";
  // }

  // errs()<<"arg" << *(pthread_cond_init->arg_begin()+1)->getType()->getPointerElementType()<<"\n";

  auto typeTest = &(*(pthread_cond_init->arg_begin() + 1)->getType()->getPointerElementType());
  auto attr_type_ptr = PointerType::get(typeTest, module->getDataLayout().getProgramAddressSpace());

  StructType *cond_type = nullptr;
  StructType *mutex_type = nullptr;

  PointerType *cond_type_ptr = nullptr;
  PointerType *mutex_type_ptr = nullptr;


  // StructType *condattr_type = nullptr;
  // StructType *mutexattr_type = nullptr;

  // PointerType *condattr_type_ptr = nullptr;
  // PointerType *mutexattr_type_ptr = nullptr;

  for (auto *st : module->getIdentifiedStructTypes()) {
    if (st->getName() == "union.pthread_cond_t") {
      // errs() << "found pthread_cond_t \n";
      cond_type = st;
    } else if (st->getName() == "union.pthread_mutex_t") {
      // errs() << "found pthread_mutex_t \n";
      mutex_type = st;
    }

    // else if (st->getName() == "union.pthread_condattr_t") {
    //   errs() << "found pthread_condattr_t \n";
    //   condattr_type = st;
    // } else if (st->getName() == "union.pthread_mutexattr_t") {
    //   errs() << "found pthread_mutexattr_t \n";
    //   mutexattr_type = st;
    // }
  }

  if (!cond_type) {
    errs() << "I was unable to locate the StructType* associated with "
              "cond_type!\n";
  } else {
    cond_type_ptr = PointerType::get(cond_type, module->getDataLayout().getProgramAddressSpace());
  }

  // if (!condattr_type) {
  //   errs() << "I was unable to locate the StructType* associated with "
  //             "condattr_type!\n";
  // } else {
  //   condattr_type_ptr = PointerType::get(condattr_type, module->getDataLayout().getProgramAddressSpace());
  // }

  if (!mutex_type) {
    errs() << "I was unable to locate the StructType* associated with "
              "mutex_type!\n";
  } else {
    mutex_type_ptr = PointerType::get(mutex_type, module->getDataLayout().getProgramAddressSpace());
  }

  // if (!mutexattr_type) {
  //   errs() << "I was unable to locate the StructType* associated with "
  //             "mutexattr_type!\n";
  // } else {
  //   mutexattr_type_ptr = PointerType::get(mutexattr_type, module->getDataLayout().getProgramAddressSpace());
  // }

  LLVMContext &context = module->getContext();

  while (true) {

    Instruction *tobeSwaped;
    Instruction *stageInfo;
    bool detectedSwap = false;
    std::string KernelName;
    // 0 for start, 1 for middle, 2 for end, -1 for non-kernel
    int stageStatusFlag = -1;
    int64_t stageIndex;
    int64_t signalNum;
    bool NoStageInfo = false;


    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;) {
      Instruction *inst = &*I;
      I++;

      if (auto *CI = dyn_cast<CallInst>(inst)) {
        Function *calledFunc = CI->getCalledFunction();
        if (calledFunc) {
          auto funcName = calledFunc->getName();
          // name start mid end
          if (funcName == "StartKernelStage") {
            stageStatusFlag = 0;
          
              auto arg = CI->arg_begin();
              const Use *use = arg;
              Value *val = use->get();
              ConstantInt *cons = dyn_cast<ConstantInt>(val);
              stageIndex = cons->getSExtValue();
              stageInfo = inst;
              tobeSwaped= &*I;
              // errs() << "arg:" << *arg << "\n";
              // errs() << "val:" << constIntValue << "\n";
            
            break;
          }
          else if (funcName == "MiddleKernelStage") {
            stageStatusFlag = 1;
            
              auto arg = CI->arg_begin();
              const Use *use = arg;
              Value *val = use->get();
              ConstantInt *cons = dyn_cast<ConstantInt>(val);
              stageIndex = cons->getSExtValue();
              stageInfo = inst;
              tobeSwaped= &*I;
              // errs() << "arg:" << *arg << "\n";
              // errs() << "val:" << constIntValue << "\n";
            
            break;
          }
          else if (funcName == "EndKernelStage") {
            stageStatusFlag = 2;
            
              auto arg = CI->arg_begin();
              const Use *use = arg;
              Value *val = use->get();
              ConstantInt *cons = dyn_cast<ConstantInt>(val);
              stageIndex = cons->getSExtValue();
              stageInfo = inst;
              tobeSwaped= &*I;
              arg++;
              use = arg;
              val = use->get();
              cons = dyn_cast<ConstantInt>(val);
              signalNum = cons->getSExtValue();

              // errs() << "CI:" << *CI << "\n";
              // errs() << "signalNum:" << signalNum << "\n";
            
            break;
          }
          else if (funcName == "SingleKernelStage") {
            stageStatusFlag = 3;
            
              auto arg = CI->arg_begin();
              const Use *use = arg;
              Value *val = use->get();
              ConstantInt *cons = dyn_cast<ConstantInt>(val);
              stageIndex = cons->getSExtValue();
              stageInfo = inst;
              tobeSwaped= &*I;
              arg++;
              use = arg;
              val = use->get();
              cons = dyn_cast<ConstantInt>(val);
              signalNum = cons->getSExtValue();            
            break;
          }
          else if (funcName == "DASH_FFT") {
            stageStatusFlag = 3;
            tobeSwaped= inst;
            signalNum = 1;
            NoStageInfo = true;
            break;
          }
        }
      }
    }

    

    
    if (stageStatusFlag == -1) {
      break;
    }

    IRBuilder<> builder(tobeSwaped);
    if (stageStatusFlag == 0||stageStatusFlag == 3) {
        // condition init
      cond_allocaMap[stageIndex] = builder.CreateAlloca(cond_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");
      auto cond_alloca = cond_allocaMap[stageIndex];
      // ConstantPointerNull *null_attrs = ConstantPointerNull::get(condattr_type_ptr);
      ConstantPointerNull *null_attrs = ConstantPointerNull::get(attr_type_ptr);

      SmallVector<Value *, 8> cond_init_args;
      cond_init_args.push_back(cond_alloca);
      cond_init_args.push_back(null_attrs);

      CallInst *cond_init = builder.CreateCall(pthread_cond_init, cond_init_args);
      cond_init->setCallingConv(pthread_cond_init->getCallingConv());


      // mutex init
      mutex_allocaMap[stageIndex] = builder.CreateAlloca(mutex_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");
      auto mutex_alloca = mutex_allocaMap[stageIndex];
      // if (mutexattr_type_ptr) {
      //   null_attrs = ConstantPointerNull::get(mutexattr_type_ptr);
      // }

      SmallVector<Value *, 8> mutex_init_args;
      mutex_init_args.push_back(mutex_alloca);
      mutex_init_args.push_back(null_attrs);

      CallInst *mutex_init = builder.CreateCall(pthread_mutex_init, mutex_init_args);
      mutex_init->setCallingConv(pthread_mutex_init->getCallingConv());


      // thread done
      threads_doneMap[stageIndex] = builder.CreateAlloca(Type::getInt32Ty(context));
      auto threads_done = threads_doneMap[stageIndex];
      StoreInst *threads_done_store = builder.CreateStore(builder.getInt32(0), threads_done);

      // mutex lock
      CallInst *mutex_lock = builder.CreateCall(pthread_mutex_lock, mutex_alloca);
      mutex_lock->setCallingConv(pthread_mutex_lock->getCallingConv());
    }
    auto *tobeSwapedCall = dyn_cast<CallInst>(tobeSwaped);
    // errs()<<"tobeSwapedCall"<<*tobeSwapedCall<<"\n";
    SmallVector<Value *, 16> kernel_args;
    StringRef myStr = tobeSwapedCall->getCalledFunction()->getName();
    Constant *kernNameConst = builder.CreateGlobalStringPtr(myStr, ".dash.kern_" + std::to_string(kern_instance_ctr));
    kernel_args.push_back(kernNameConst);

    // Propagate the rest of the arguments from the original call
    
    // errs()<<"call" <<*tobeSwapedCall<<"\n";
    for (const auto &arg : tobeSwapedCall->args()) {
      const Use &use = arg;
      Value *val = use.get();
      // To get "pointers" to the values that were originally passed in, we just make a new Alloca here
      // And then we store the current value in that memory address
      // Thinking through cases where the original arg was either a scalar or a pointer,
      // I can't think of any edge cases where this would go wrong (?)
      AllocaInst *pointerAI = builder.CreateAlloca(val->getType());
      StoreInst *pointerStore = builder.CreateStore(val, pointerAI);

      kernel_args.push_back(pointerAI);
    }
    kernel_args.push_back(cond_allocaMap[stageIndex]);
    kernel_args.push_back(mutex_allocaMap[stageIndex]);
    kernel_args.push_back(threads_doneMap[stageIndex]);

    CallInst *ReplacementCI = builder.CreateCall(cedr_dispatch, kernel_args);
    ReplacementCI->setCallingConv(cedr_dispatch->getCallingConv());

     if (stageStatusFlag == 2||stageStatusFlag == 3) {
         BasicBlock *parent = ReplacementCI->getParent();

      BasicBlock *newBB = parent->splitBasicBlock(ReplacementCI->getNextNode(), "splitBB");
        if (newBB->getInstList().size() > 1) {
                  newBB->splitBasicBlock(newBB->front().getNextNode());
              }
      // errs() << "module" << *module;
      //   errs()<<"ReplacementCI"<<*ReplacementCI<<"\n";
      //   errs()<<"newBB"<<*newBB;

      BranchInst* parentBR = dyn_cast<BranchInst> (parent->getTerminator());

      BasicBlock* cmp = BasicBlock::Create(context,"cmp", &F);
      BasicBlock* condWaitingBB = BasicBlock::Create(context,"condWait", &F);
      BasicBlock* mutexUnlockBB = BasicBlock::Create(context,"mutexUnlock", &F);

      parentBR->	setSuccessor (0, cmp);

      builder.SetInsertPoint(cmp);

      LoadInst *load_counter = builder.CreateLoad(threads_doneMap[stageIndex]);

      // to do 2 need to be changed to be the same as the counter
      Value* notEqual2 = builder.CreateICmpNE(load_counter, builder.getInt32(signalNum), "tmp");
      builder.CreateCondBr(notEqual2, condWaitingBB, mutexUnlockBB);

      // waiting bb
      builder.SetInsertPoint(condWaitingBB);
      SmallVector<Value *, 8> cond_wait_args;
      cond_wait_args.push_back(cond_allocaMap[stageIndex]);
      cond_wait_args.push_back(mutex_allocaMap[stageIndex]);

      CallInst *CondWait = builder.CreateCall(pthread_cond_wait, cond_wait_args);
      CondWait->setCallingConv(pthread_cond_wait->getCallingConv());
      builder.CreateBr(cmp);

      // unlock bb
      builder.SetInsertPoint(mutexUnlockBB);
      // unlock the mutex in a new basic block
      CallInst *mutex_unlock = builder.CreateCall(pthread_mutex_unlock, mutex_allocaMap[stageIndex]);
      mutex_unlock->setCallingConv(pthread_mutex_unlock->getCallingConv());
      builder.CreateBr(newBB);
     
     
     }
    
    
    // // if (EndKernelIndexToCounter.find(CurrentKernelIndexInsideStage) !=EndKernelIndexToCounter.end()) {
    //   if (CurrentKernelIndexInsideStage==2) {
    //   // split the old basic block

    

    //   // set inst counter to 0

    // }
    // errs()<<"bb"<<  *(tobeSwaped->getParent()) <<"\n";
    
    tobeSwaped->eraseFromParent();
    if (!NoStageInfo) {
      stageInfo->eraseFromParent();
    }
    
    kern_instance_ctr++;
    
    // // errs() << "module" << *module;
  }

  // Before exiting, go through and erase all KernelEnter and KernelExit calls
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;) {
    Instruction *inst = &*I;
    I++;

    if (auto *CI = dyn_cast<CallInst>(inst)) {
      Function *calledFunc = CI->getCalledFunction();

      if (calledFunc) {
        auto funcName = calledFunc->getName();
        // name start mid
        if (funcName == "KernelEnter" || funcName == "KernelExit"|| funcName == "LoadDump"|| funcName == "NonKernelSplit"|| funcName == "LoadDump"|| funcName == "BB_ID_Dump"|| funcName == "StoreDump"|| funcName == "MemCpyDump") {
          inst->eraseFromParent();
        }
      }
    }
  }

  modified = true;
  return modified;
}

bool SwapAPIs::doInitialization(Module &M) {
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

char SwapAPIs::ID = 0;
static RegisterPass<SwapAPIs> Z("SwapAPIs",
                                "Find instances of DASH API calls throughout a given set of LLVM modules "
                                "and swap them for CEDR invocations",
                                false, false);
