#include "Passes/KernelStage.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"
#include <fstream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

map<int64_t, int64_t> startKernelIndexTrans;
map<int64_t, int64_t> middleKernelIndexTrans;
map<int64_t, pair<int64_t, int64_t>> EndKernelIndexToCounterTrans;
namespace DashTracer::Passes
{
    // source node end bb -> old target start bb, new target start bb

    bool KernelStage::runOnFunction(Function &F)
    {
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
        {
            auto *block = cast<BasicBlock>(BB);
            auto dl = block->getModule()->getDataLayout();
            int64_t blockId = GetBlockID(block);

            if  (startKernelIndexTrans.find(blockId) != startKernelIndexTrans.end() && EndKernelIndexToCounterTrans.find(blockId) != EndKernelIndexToCounterTrans.end())
            {
                std::vector<Value *> args;
                Value *StageValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)EndKernelIndexToCounterTrans[blockId].first);
                args.push_back(StageValue);
                Value *numValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)EndKernelIndexToCounterTrans[blockId].second);
                args.push_back(numValue);

                for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
                {
                    if (auto *CI = dyn_cast<CallInst>(BI))
                    {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc->getName() == "DASH_FFT"||calledFunc->getName() == "DASH_GEMM")
                        {
                            IRBuilder<> Builder(CI);
                            Builder.CreateCall(SingleKernelStage, args);
                            break;
                        }
                    }
                }                
                // errs() << "bb:" << *block << "\n";
            }
            else if (startKernelIndexTrans.find(blockId) != startKernelIndexTrans.end())
            {

                std::vector<Value *> args;
                Value *StageValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)startKernelIndexTrans[blockId]);
                args.push_back(StageValue);
                for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
                {
                    if (auto *CI = dyn_cast<CallInst>(BI))
                    {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc->getName() == "DASH_FFT"||calledFunc->getName() == "DASH_GEMM")
                        {
                            IRBuilder<> Builder(CI);
                            Builder.CreateCall(StartKernelStage, args);
                            break;
                        }
                    }
                }
                // errs() << "bb:" << *block << "\n";
                // call kernelstagestart
            }else if (EndKernelIndexToCounterTrans.find(blockId) != EndKernelIndexToCounterTrans.end())
            {
                // call kernelstagestart
                std::vector<Value *> args;
                Value *StageValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)EndKernelIndexToCounterTrans[blockId].first);
                args.push_back(StageValue);
                Value *numValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)EndKernelIndexToCounterTrans[blockId].second);
                args.push_back(numValue);

                for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
                {
                    if (auto *CI = dyn_cast<CallInst>(BI))
                    {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc->getName() == "DASH_FFT"||calledFunc->getName() == "DASH_GEMM")
                        {
                            IRBuilder<> Builder(CI);
                            Builder.CreateCall(EndKernelStage, args);
                            break;
                        }
                    }
                }                
                // errs() << "bb:" << *block << "\n";
            }

            if (middleKernelIndexTrans.find(blockId) != middleKernelIndexTrans.end())
            {
              
                std::vector<Value *> args;
                Value *StageValue = ConstantInt::get(Type::getInt64Ty(BB->getContext()), (uint64_t)middleKernelIndexTrans[blockId]);
                args.push_back(StageValue);

                for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
                {
                    if (auto *CI = dyn_cast<CallInst>(BI))
                    {
                        Function *calledFunc = CI->getCalledFunction();
                        if (calledFunc->getName() == "DASH_FFT"||calledFunc->getName() == "DASH_GEMM")
                        {
                            IRBuilder<> Builder(CI);
                            Builder.CreateCall(MiddleKernelStage, args);
                            break;
                        }
                    }
                } 
                // errs() << "bb:" << *block << "\n";
            }
        }
        return true;
    }

    bool KernelStage::doInitialization(Module &M)
    {
        nlohmann::json j;
        std::ifstream inputStream(DAGBBIOFile);
        inputStream >> j;
        inputStream.close();
        nlohmann::json mapping;
        mapping = j["EndKernelIndexToCounter"];
        EndKernelIndexToCounterTrans = mapping.get<map<int64_t, pair<int64_t, int64_t>>>();
        nlohmann::json setting = j["startKernelIndex"];
        startKernelIndexTrans = setting.get<map<int64_t, int64_t>>();
        setting = j["middleKernelIndex"];
        middleKernelIndexTrans = setting.get<map<int64_t, int64_t>>();

        StartKernelStage = cast<Function>(M.getOrInsertFunction("StartKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());

        EndKernelStage = cast<Function>(M.getOrInsertFunction("EndKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext()),Type::getInt64Ty(M.getContext())).getCallee());
        SingleKernelStage = cast<Function>(M.getOrInsertFunction("SingleKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext()),Type::getInt64Ty(M.getContext())).getCallee());
        MiddleKernelStage = cast<Function>(M.getOrInsertFunction("MiddleKernelStage", Type::getVoidTy(M.getContext()), Type::getInt64Ty(M.getContext())).getCallee());

        return false;
    }

    void KernelStage::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char KernelStage::ID = 1;
    static RegisterPass<KernelStage> Y("KernelStage", "add kernel stage information", true, false);
} // namespace DashTracer::Passes