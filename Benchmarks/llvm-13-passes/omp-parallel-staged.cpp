#include "llvm/Frontend/OpenMP/OMP.h.inc"
#include "llvm/Frontend/OpenMP/OMPConstants.h"
#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <nlohmann/json.hpp>
#include <forward_list>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>

using namespace llvm;
using namespace std;

struct OmpParallel : public FunctionPass {
  static char ID;
  OmpParallel() : FunctionPass(ID) { kern_instance_ctr = 0; }
  auto runOnFunction(Function &F) -> bool override;
  auto doInitialization(Module &M) -> bool override;

private:
  uint64_t kern_instance_ctr;
};

cl::opt<std::string> DAGInfo("dg", cl::desc("dag information file"), cl::value_desc("dag information file filename"), cl::Required);
map<int, set<int>> nonkernelStage;
map<string, pair<int, int>> NodeIO;
map<int, BasicBlock *> BBIDMap;

auto OmpParallel::runOnFunction(Function &F) -> bool {

  using InsertPointTy = OpenMPIRBuilder::InsertPointTy;
  using BodyGenCallbackTy = llvm::OpenMPIRBuilder::StorableBodyGenCallbackTy;
  auto *M = F.getParent();
  OpenMPIRBuilder OMPBuilder(*M);
  OMPBuilder.initialize();

  bool modified = false;
  if (F.getName() != "main") {
    return modified;
  }
  IRBuilder<> Builder(&(F.getEntryBlock())); 

  Instruction* testStart;
  Function* testFunc;
  Value* argment;
  for (auto &BB : F) {
    for (auto &inst :   BB)
    {
      if (auto *CI = dyn_cast<CallInst>(&inst))
      {
          Function *calledFunc = CI->getCalledFunction();
          // add to each non kernel stage start, stage size
          if (calledFunc->getName() == "nk_1")
          {
            testFunc = calledFunc;
            testStart = &inst;
            argment = CI->getOperand(0);
            // errs()<< *argment;
          }
      }
    } 
  }


  auto PrivCB = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, Value &Orig, Value &Inner, Value *&ReplacementValue) -> InsertPointTy { 


    if (!isa<AllocaInst>(Orig)) {
      ReplacementValue = &Inner;
      return CodeGenIP;
    }


    // Builder.restoreIP(AllocaIP);
    // Type *VTy = ReplacementValue->getType();
    // Value *V = Builder.CreateLoad(VTy, &Inner, Orig.getName() + ".reload");
    // ReplacementValue = Builder.CreateAlloca(VTy, 0, Orig.getName() + ".copy");
    // Builder.restoreIP(CodeGenIP);
    // Builder.CreateStore(V, ReplacementValue);


    Builder.restoreIP(AllocaIP);
    Type *VTy = Inner.getType()->getPointerElementType();
    Value *V = Builder.CreateLoad(VTy, &Inner, Orig.getName() + ".reload");
    ReplacementValue = Builder.CreateAlloca(VTy, 0, Orig.getName() + ".copy");
    Builder.CreateStore(V, ReplacementValue);
    
    
    Builder.restoreIP(CodeGenIP);
    


    errs()<<"innner:"<<Inner<<"\n";
    errs()<<"Orig:"<<Orig<<"\n";
    errs()<<"ReplacementValue:"<<*ReplacementValue<<"\n";



    
    return CodeGenIP;
  };


  auto BodyGenCB = [&,argment](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, BasicBlock &ContinuationIP) {
    SmallVector<BodyGenCallbackTy, 4> SectionCBVector;

    auto FiniCB = [&](InsertPointTy IP) {};


    auto SectionCB = [&,argment](InsertPointTy AllocaIPSec, InsertPointTy CodeGenIPSec, BasicBlock &FiniBB) {
      
      Builder.restoreIP(CodeGenIPSec);
     
      Builder.CreateCall(testFunc,argment);

      Builder.CreateBr(&FiniBB);
      // errs()<< *(CodeGenIPSec.getBlock());
    };
    SectionCBVector.push_back(SectionCB);
    // break;

    // auto PrivCB = [](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, llvm::Value &, llvm::Value &Val, llvm::Value *&ReplVal) {


    //   Builder.restoreIP(AllocaIP);
    //   Type *VTy = Inner.getType()->getPointerElementType();
    //   Value *V = Builder.CreateLoad(VTy, &Inner, Orig.getName() + ".reload");
    //   ReplacementValue = Builder.CreateAlloca(VTy, 0, Orig.getName() + ".copy");
    //   Builder.restoreIP(CodeGenIP);
    //   Builder.CreateStore(V, ReplacementValue);

    //    return CodeGenIP; 
    // };

    auto *CodeGenBB = CodeGenIP.getBlock();

    auto *newBB = SplitBlock(CodeGenBB, &*CodeGenIP.getPoint());
    CodeGenBB->getTerminator()->eraseFromParent();

    Builder.SetInsertPoint(CodeGenBB);
    IRBuilder<>::InsertPoint AfterIP = OMPBuilder.createSections(InsertPointTy(CodeGenBB, CodeGenBB->end()), AllocaIP, SectionCBVector, PrivCB, FiniCB, false, false);

    Builder.restoreIP(AfterIP);
    // branch back to the original target at the end of create parallel call
    Builder.CreateBr(newBB);
    
  };

  // AllocaInst *PrivAI = nullptr;

  auto BodyGenCB2 = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, BasicBlock &ContinuationIP) {
    Builder.restoreIP(CodeGenIP);
    // errs()<<"test1\n"; 
    // /Builder.CreateCall(testFunc,testStart->getOperand(0));
    // PrivAI = Builder.CreateAlloca(testStart->getOperand(0)->getType());
    // Builder.CreateStore(testStart->getOperand(0), PrivAI);

    // SmallVector<Value *, 8> args;
    //   args.push_back(argment);
    //   // errs()<<*argment<<"\n";
    // auto *allocatest= dyn_cast<AllocaInst>(argment);
    // errs()<< *allocatest;
    // Value *PrivLoad = Builder.CreateLoad(allocatest->getAllocatedType(), argment, "local.use.test");

      


    // Builder.CreateCall(testFunc,args);
    
    //errs() << *CodeGenIP.getBlock()->getParent();
    Builder.CreateCall(testFunc,argment);

  };



  auto FiniCB = [&](InsertPointTy CodeGenIP) {

    bool verifyRes = verifyFunction(F);
    errs() << "verfiy final:" <<verifyRes<<"\n"; 
  };

  auto* currentBlock= testStart->getParent();
  auto* aftersplit =  currentBlock->splitBasicBlock(testStart);
  aftersplit->getTerminator()->eraseFromParent();
  
  IRBuilder<>::InsertPoint AllocaIP(currentBlock, currentBlock->getFirstInsertionPt());
  Builder.SetInsertPoint(testStart);
  IRBuilder<>::InsertPoint AfterIP = OMPBuilder.createParallel(Builder.saveIP(), AllocaIP, BodyGenCB, PrivCB, FiniCB, nullptr, nullptr, llvm::omp::OMP_PROC_BIND_default, false);

  bool verifyRes = verifyFunction(F);
    errs() << "verfiy final2:" <<verifyRes<<"\n"; 

  Builder.restoreIP(AfterIP);
  
  Builder.CreateBr(aftersplit);

  OMPBuilder.finalize();

  // errs() << "module" << *M << "\n";

  modified = true;
  return modified;
}

auto OmpParallel::doInitialization(Module &M) -> bool {
  bool modified = false;
  nlohmann::json k;
  std::ifstream inputStreamDAG(DAGInfo);
  inputStreamDAG >> k;
  inputStreamDAG.close();
  nlohmann::json DAGmapping;
  DAGmapping = k["nonkernelStage"];
  nonkernelStage = DAGmapping.get<map<int, set<int>>>();
  nlohmann::json NodeIOjs;
  NodeIOjs = k["NodeIO"];
  NodeIO = NodeIOjs.get<map<string, pair<int, int>>>();

  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &inst : BB) {
        if (auto *CI = dyn_cast<CallInst>(&inst)) {
          Function *calledFunc = CI->getCalledFunction();
          if (calledFunc->getName() == "BB_ID_Dump") {
            auto *cons = dyn_cast<ConstantInt>(CI->arg_begin()->get());
            auto number = (uint64_t)cons->getSExtValue();
            // get the id
            // errs()<<number<<","<<&BB<<"\n";
            BBIDMap[int(number)] = &BB;
            break;
          }
        }
      }
    }
  }

  return modified;
}

char OmpParallel::ID = 0;
static RegisterPass<OmpParallel> X("OmpParallel",
                                   "Find instances of DASH API calls throughout a given set of LLVM modules "
                                   "and swap them for CEDR invocations",
                                   false, false);
