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
cl::opt<std::string> DAGDetect("jr", cl::desc("jr information file"), cl::value_desc("dag information file filename"), cl::Required);
map<int, set<int>> nonkernelStage;
map<string, pair<int, int>> NodeIO;
map<int, BasicBlock *> BBIDMap;
map<int, set<int>> kernelControlMap;

auto OmpParallel::runOnFunction(Function &F) -> bool {


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



  using InsertPointTy = OpenMPIRBuilder::InsertPointTy;
  using BodyGenCallbackTy = llvm::OpenMPIRBuilder::StorableBodyGenCallbackTy;
  auto *M = F.getParent();
  OpenMPIRBuilder OMPBuilder(*M);
  OMPBuilder.initialize();

  bool modified = false;
  if (F.getName() != "main") {
    return modified;
  }

  IRBuilder<> Builder(&F.getEntryBlock());

  for (auto stage : nonkernelStage) {
    if (stage.second.size() < 2) {
      // a not valid nonkernel stage, node number not enough
      continue;
    }

    bool eligibleForExtract = true;

    // check if all nodes in stage are extractable

    for (auto node : stage.second) {
      vector<BasicBlock *> Blocks;
      // int startID = *(stage.second.begin());
      // int endID = *(stage.second.rbegin());
      // int counter = 0;
      // bool startStageNode = false;
      // bool endStageNode = false;
      // if (node == startID)
      // {
      //   Blocks.push_back(StartStageInRegion);
      //   startStageNode = true;
      // }
      //   for (auto block : kernelControlMap[node]) {
      //   if (block ==  NodeIO[to_string(startID)].first && startStageNode) {
      //     continue;
      //   }
      //   if (block ==  NodeIO[to_string(endID)].second && endNodeInStage) {
      //     continue;
      //   }
      //   Blocks.push_back(BBIDMap[block]);
      // }
      // if (node == endID)
      // {
      //   Blocks.push_back(endNodeInStage);
      //   endStageNode = true;
      // }

      for (auto block : kernelControlMap[node]) {
        // errs()<<"block,"<<BBIDMap[block]<<"\n";

        if(BBIDMap[block]->getParent()->getName() =="main")
      {      
        Blocks.push_back(BBIDMap[block]);
        // errs()<<*(BBIDMap[block])<<"\n";
      }
      }
      CodeExtractorAnalysisCache CEAC(F);
      CodeExtractor Extractor(Blocks);

      // CodeExtractor Extractor(Blocks,NULL,true,NULL, NULL,NULL, false,false);

      // Function *Outlined = Extractor.extractCodeRegion(CEAC); 


      errs() << " ce eligible: " << static_cast<int>(Extractor.isEligible()) << "\n";
      eligibleForExtract = Extractor.isEligible();

      if (eligibleForExtract == false) {
        errs() << " not eligible, node id "<< node <<"\n";
        break;
      }
      // BasicBlock *CommonExit = nullptr;
      // SetVector<Value *> Inputs, Outputs, SinkingCands, HoistingCands;
      // Extractor.findAllocas(CEAC, SinkingCands, HoistingCands, CommonExit);
      // Extractor.findInputsOutputs(Inputs, Outputs, SinkingCands);
    }

    if (eligibleForExtract == false) {
      continue;
    }
    errs() << "eligible stage: " << stage.first << "\n";

    BasicBlock *startNodeInStage = BBIDMap[NodeIO[to_string(*(stage.second.begin()))].first];

    // BasicBlock* startNodeInStage = BBIDMap[NodeIO[to_string(7)].first];

    BasicBlock *endNodeInStage = BBIDMap[NodeIO[to_string(*(stage.second.rbegin()))].second];

    auto *StartStageInRegion = startNodeInStage->splitBasicBlock(startNodeInStage->begin());
    startNodeInStage->getTerminator()->eraseFromParent();

    // auto * StartStageInRegion = startNodeInStage;

    // auto *EndStageOutRegion = endNodeInStage;
    auto *EndStageOutRegion = endNodeInStage->splitBasicBlock(endNodeInStage->getTerminator());


    bool test = true;
    if (test) {

      auto PrivCB = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, Value &Orig, Value &Inner, Value *&ReplacementValue) -> InsertPointTy {

        ReplacementValue = &Inner;
        // if (!isa<AllocaInst>(Orig)) {
        //   ReplacementValue = &Inner;
        //   return CodeGenIP;
        // }

        // Builder.restoreIP(AllocaIP);
        // Type *VTy = Inner.getType()->getPointerElementType();
        // Value *V = Builder.CreateLoad(VTy, &Inner, Orig.getName() + ".reload");
        // ReplacementValue = Builder.CreateAlloca(VTy, 0, Orig.getName() + ".copy");
        // Builder.CreateStore(V, ReplacementValue);

        // Builder.restoreIP(CodeGenIP);

        // errs()<<"innner:"<<Inner<<"\n";
        // errs()<<"Orig:"<<Orig<<"\n";
        // errs()<<"ReplacementValue:"<<*ReplacementValue<<"\n";
        return CodeGenIP;
      };

      auto BodyGenCB2 = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, BasicBlock &ContinuationIP) {
        SmallVector<BodyGenCallbackTy, 4> SectionCBVector;

        auto FiniCB = [&](InsertPointTy IP) {};

        // the nodes for stage
        int startStageNodeID = *(stage.second.begin());
        int endStageNodeID = *(stage.second.rbegin());

        // wrap up one node with bbs in a section
        int counter = 0;
        for (auto node : stage.second) {

          BasicBlock *startBBinNode = nullptr;
          BasicBlock *endBBinNode = nullptr;

          // fix the case that we split the start and end BB in each stage
          startBBinNode = BBIDMap[NodeIO[to_string(node)].first];
          // errs()<< startBBinNode<<"\n";
          if (node == startStageNodeID) {
            startBBinNode = StartStageInRegion;
          }

          endBBinNode = BBIDMap[NodeIO[to_string(node)].second];
          if (node == endStageNodeID) {
            // end node
            endBBinNode = endNodeInStage;
          }

          counter++;
          // if (counter==2||counter==3||counter ==4)
          // {
          //   continue;;
          // }

          auto SectionCB = [&, startBBinNode, endBBinNode](InsertPointTy AllocaIPSec, InsertPointTy CodeGenIPSec, BasicBlock &FiniBB) {
            Builder.restoreIP(CodeGenIPSec);

            Builder.CreateBr(startBBinNode);

            // auto *CodeGenBB = CodeGenIP.getBlock();

            // auto *newBB = CodeGenBB->splitBasicBlock(CodeGenBB->getTerminator());
            // CodeGenBB->getTerminator()->eraseFromParent();
            // Builder.SetInsertPoint(CodeGenBB);

            // // errs()<<"codegen" <<*CodeGenBB;
            // Builder.CreateBr(startBBinNode);

            // auto* test = endBBinNode;

            // Builder.SetInsertPoint(test);
            // test->getTerminator()->eraseFromParent();
            // Builder.CreateBr(newBB);

            // errs()<<*endBBinNode;
            // errs()<< "n of ter: "<<endBBinNode->getTerminator()->getNumSuccessors() <<"\n";
            endBBinNode->getTerminator()->eraseFromParent();
            Builder.SetInsertPoint(endBBinNode);
            Builder.CreateBr(&FiniBB);
            // errs()<<"in," << *endBBinNode<<"\n";
            // errs()<<F<<"\n";
          };
          SectionCBVector.push_back(SectionCB);
          // SectionCBVector.push_back(SectionCB);
          // break;
        }
        // break;

        auto *CodeGenBB = CodeGenIP.getBlock();

        auto *orignal = CodeGenBB->getSingleSuccessor();
        CodeGenBB->getTerminator()->eraseFromParent();

        Builder.SetInsertPoint(CodeGenBB);
        IRBuilder<>::InsertPoint AfterIP = OMPBuilder.createSections(Builder.saveIP(), AllocaIP, SectionCBVector, PrivCB, FiniCB, false, false);

        Builder.restoreIP(AfterIP);
        // branch back to the original target at the end of create parallel call
        Builder.CreateBr(orignal);
        // errs() << *M;
      };

      // SmallPtrSet<BasicBlock *, 32> BlockSet;
      // SmallVector<BasicBlock *, 32> BlocksVect;
      // SmallVector<BasicBlock *, 32> Worklist;
      // BlockSet.insert(StartStageInRegion);
      // BlockSet.insert(BBIDMap[NodeIO["7"].second]);

      // Worklist.push_back(StartStageInRegion);
      // while (!Worklist.empty()) {

      //   BasicBlock *BB = Worklist.pop_back_val();
      //   // errs()<<*BB;
      //   BlocksVect.push_back(BB);
      //   for (BasicBlock *SuccBB : successors(BB))
      //     if (BlockSet.insert(SuccBB).second)
      //       Worklist.push_back(SuccBB);
      // }

      // CodeExtractorAnalysisCache CEAC(F);
      // CodeExtractor Extractor1(BlocksVect);
      // errs() << " 1111 ce eligible: " << static_cast<int>(Extractor1.isEligible()) << "\n";

      // auto* target = BBIDMap[NodeIO["1"].second]->getSingleSuccessor();

      auto BodyGenCB = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP, BasicBlock &ContinuationIP) {
        BasicBlock *startBBinNode;
        BasicBlock *endBBinNode;

        startBBinNode = StartStageInRegion;

        endBBinNode = BBIDMap[NodeIO["1"].second];

        auto *CodeGenBB = CodeGenIP.getBlock();

        auto *newBB = CodeGenBB->splitBasicBlock(CodeGenBB->getTerminator());
        CodeGenBB->getTerminator()->eraseFromParent();
        Builder.SetInsertPoint(CodeGenBB);

        // errs()<<"codegen" <<*CodeGenBB;
        Builder.CreateBr(startBBinNode);

        // auto *test = startBBinNode->getTerminator()->getSuccessor(0);
        auto *test = endBBinNode;

        // errs()<<*test;

        // errs()<< *BBIDMap[85];

        Builder.SetInsertPoint(test);
        test->getTerminator()->eraseFromParent();
        Builder.CreateBr(newBB);
        // errs()<< *M;

        // branch back to the original target at the end of create parallel call
        // Builder.CreateBr(newBB);

        // errs()<<*M<<"\n";
        // endBBinNode->getTerminator()->eraseFromParent();
        // Builder.SetInsertPoint(endBBinNode);
      };

      auto FiniCB = [&](InsertPointTy CodeGenIP) {};

      Builder.SetInsertPoint(startNodeInStage);
      IRBuilder<>::InsertPoint StartPoint(startNodeInStage, startNodeInStage->getFirstInsertionPt());

      int size = stage.second.size();
      // auto *ThreadNum = ConstantInt::get(F.getContext(), llvm::APInt(32, (uint64_t)size, true));
      auto *ThreadNum = ConstantInt::get(F.getContext(), llvm::APInt(32, (uint64_t)size, true));
      IRBuilder<>::InsertPoint AfterIP =
          OMPBuilder.createParallel(Builder.saveIP(), StartPoint, BodyGenCB2, PrivCB, FiniCB, nullptr, ThreadNum, llvm::omp::OMP_PROC_BIND_spread, false);

      Builder.restoreIP(AfterIP);

      // branch back to the original target at the end of create parallel call
      Builder.CreateBr(EndStageOutRegion);

          // errs()<<"end stage:"<<*BBIDMap[NodeIO[to_string(1)].second]<<"\n";
    //  errs()<<"next:"<<*target<<"\n";

      // Builder.CreateBr(target);
      
      OMPBuilder.finalize();

      // break;
    }
  }

  modified = true;
  return modified;
}

auto OmpParallel::doInitialization(Module &M) -> bool {
  bool modified = false;
  nlohmann::json k;
  std::ifstream inputStreamDAG(DAGInfo);
  std::ifstream inputStreamDAGNode(DAGDetect);
  inputStreamDAG >> k;
  inputStreamDAG.close();
  nlohmann::json DAGmapping;
  DAGmapping = k["nonkernelStage"];
  nonkernelStage = DAGmapping.get<map<int, set<int>>>();
  nlohmann::json NodeIOjs;
  NodeIOjs = k["NodeIO"];
  NodeIO = NodeIOjs.get<map<string, pair<int, int>>>();

  nlohmann::json d;
  inputStreamDAGNode >> d;
  inputStreamDAGNode.close();

  for (auto &[k, l] : d["Control"].items()) {
    int index = stoul(k, nullptr, 0);
    nlohmann::json kernel = l["Blocks"];
    kernelControlMap[index] = kernel.get<set<int>>();
  }

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
