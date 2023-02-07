#include "Passes/SwapNonkernel.h"
#include "AtlasUtil/Annotate.h"
#include "Backend/BackendTrace.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include <cstdint>
#include <llvm-9/llvm/IR/Instruction.h>
#include <llvm-9/llvm/IR/Value.h>
#include <llvm-9/llvm/Support/Casting.h>
#include <llvm-9/llvm/Support/raw_ostream.h>
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

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SourceMgr.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <sys/prctl.h>
#include <unistd.h>
#include <vector>
#include <regex>

using namespace llvm;
using namespace std;


map<int,set<int>> nonkernelStage;
bool DashTracer::Passes::SwapNonkernel::runOnFunction(Function &F) {
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

    StructType *att_type = nullptr;
    // PointerType *att_type_ptr = nullptr;



  for (auto *st : module->getIdentifiedStructTypes()) {
    if (st->getName() == "union.pthread_attr_t") {
      att_type = st;
    } 
  }

  if (!att_type) {
    errs() << "I was unable to locate the StructType* associated with "
              "att_type!\n";
  } 
//   else {
//     att_type_ptr = PointerType::get(att_type, module->getDataLayout().getProgramAddressSpace());
//   }

  // used for pthread function
  Constant *funcPtr;
  Function *nkFunc;
  Value* argment;
  AllocaInst* ThreadAlloca;

//  function name -> thread variable ID
  map<string, int> functionThreadID;
  
    for (auto & BB : F)
  {

    for (BasicBlock::iterator BI = BB.begin(), BE = BB.end(); BI != BE; ++BI)
    {
        if (auto *CI = dyn_cast<CallInst>(BI))
        {
            Function *calledFunc = CI->getCalledFunction();
            if (calledFunc->getName() =="MainStart")
            {
                // add thread id array
                IRBuilder<> builder(CI);
                Type* I = IntegerType::getInt64Ty(module->getContext());              
                auto *cons = dyn_cast<ConstantInt>(CI->arg_begin()->get());
                auto num = (uint64_t) cons->getSExtValue();
                
                ArrayType* arrayType = ArrayType::get(I, num);

                ThreadAlloca  = builder.CreateAlloca(arrayType);
                
                // todo  delete dummy function
                // CI->eraseFromParent();
                // calledFunc->eraseFromParent();

            }
            if (calledFunc->getName() =="StartNonKernelStage")
            {

                auto *cons = dyn_cast<ConstantInt>(CI->arg_begin()->get());
                auto number = (uint64_t) cons->getSExtValue();


                int indexInStage = 0;
                for (auto nk : nonkernelStage[int(number)]) {
                    string name = "nk_"+to_string(nk);
                    functionThreadID[name] = indexInStage;
                    // errs()<<"name: " <<name << " index in stage: "<< indexInStage <<"\n";
                    indexInStage++;
                }
                // todo  delete dummy function
                // CI->eraseFromParent();
                // calledFunc->eraseFromParent();

            }
            if (calledFunc->getName() =="EndNonKernelStage")
            {
                auto *cons = dyn_cast<ConstantInt>(CI->arg_begin()->get());
                auto number = (uint64_t) cons->getSExtValue();
                uint64_t stageSize = nonkernelStage[int(number)].size();
                for (uint64_t i=0; i< stageSize; i++) {
                

                    // create pthread join calls              
                    IRBuilder<> builder(CI->getNextNode());
                    SmallVector<Value *, 8> pthread_join_args;
                    auto *zero = ConstantInt::get(BB.getContext(), llvm::APInt(32, 0, true));
                    auto *index = ConstantInt::get(BB.getContext(), llvm::APInt(32, i, true));
                    Type *VoidPointerType = Type::getInt8PtrTy(module->getContext());
                    auto *nulltype = ConstantPointerNull::get(PointerType::get(VoidPointerType, 0));
                    
                    auto *threadIDPtr = builder.CreateGEP(ThreadAlloca, { zero, index });
                    auto *loadThreadID = builder.CreateLoad(Type::getInt64Ty(module->getContext()),threadIDPtr);

                    pthread_join_args.push_back(loadThreadID);
                    pthread_join_args.push_back(nulltype);

                    builder.CreateCall(pthread_join, pthread_join_args);
                }
                //add pthread join
                // CI->eraseFromParent();
                // calledFunc->eraseFromParent();
            }
        }
    }
  }
  
  regex pattern("^nk_.*$");
  for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
  {
    for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
    {
        if (auto *CI = dyn_cast<CallInst>(BI))
        {
            Function *calledFunc = CI->getCalledFunction();
            auto name = string(calledFunc->getName());
            auto words_begin = sregex_iterator(name.begin(), name.end(), pattern);
            auto words_end = sregex_iterator();
            
            
            if (distance(words_begin, words_end)!= 0)
            {
                // const smatch& match = *words_begin;
                nkFunc = calledFunc;
                // int8 pointer is the typical pointer type
                auto *FT = FunctionType::get(Type::getInt8PtrTy(module->getContext()), Type::getInt8PtrTy(module->getContext()), false);
                
                funcPtr =  ConstantExpr::getBitCast(calledFunc, PointerType::get(FT, 0));
                //nkFunc->mutateType(FT);
                argment =  CI->getArgOperand(0);

                
                IRBuilder<> builder(CI->getNextNode());
                SmallVector<Value *, 8> pthread_create_args;


                int ThreadIDIndex = functionThreadID[name];

                auto *zero = ConstantInt::get(BB->getContext(), llvm::APInt(32, 0, true));
                auto *index = ConstantInt::get(BB->getContext(), llvm::APInt(32, (uint64_t)ThreadIDIndex, true));
                auto *nulltype = ConstantPointerNull::get(PointerType::get(att_type, 0));
                
                // index list, first is the pointer address offset, second is the array index
                auto *threadIDPtr = builder.CreateGEP(ThreadAlloca, { zero, index });
                // Value * value = ConstantInt::get(BB->getContext(), llvm::APInt(64, 5, true));
                // builder.CreateStore(value, threadIDPtr);

                pthread_create_args.push_back(threadIDPtr);
                pthread_create_args.push_back(nulltype);


                pthread_create_args.push_back(funcPtr);

                // IRBuilder<> Builder(CI->getNextNode());
                Type *VoidPointerTy = Type::getInt8PtrTy(module->getContext());
                auto *argmentNK = builder.CreateBitCast(argment, VoidPointerTy);

                pthread_create_args.push_back(argmentNK);

                // errs()<<"type:"<<*(funcPtr->getType()) <<"\n";
                // pthread_create_args.push_back(nulltype);
                // thread id, 
                builder.CreateCall(pthread_create, pthread_create_args);

                //funcPtr =  ConstantExpr::getBitCast(calledFunc, Type::getInt8PtrTy(module->getContext()));
                // errs()<<"number arg:"<< *(calledFunc->arg_begin())<<"\n";
            }
        }

    }
  }



 
  errs()<< "pass";
  modified = true;
  return modified;
}

bool DashTracer::Passes::SwapNonkernel::doInitialization(Module &M) {
  bool modified = false;
  for (Module::iterator F = M.begin(), E = M.end(); F != E;) {
    Function *func = &*F;
    F++;
    if (func->getName() == "dummy_function") {
      func->eraseFromParent();
      modified = true;
    }
  }

  nlohmann::json k;
    std::ifstream inputStreamDAG(DAGBBIOFile);
    inputStreamDAG >> k;
    inputStreamDAG.close();
    nlohmann::json DAGmapping;
    DAGmapping = k["nonkernelStage"];
    nonkernelStage = DAGmapping.get<map<int,set<int>>>();



  return modified;
}
void DashTracer::Passes::SwapNonkernel::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
    AU.setPreservesCFG();
}

char DashTracer::Passes::SwapNonkernel::ID = 0;
static RegisterPass<DashTracer::Passes::SwapNonkernel> Z("SwapNonkernel",
                                "Find instances of DASH API calls throughout a given set of LLVM modules "
                                "and swap them for CEDR invocations",
                                false, false);
