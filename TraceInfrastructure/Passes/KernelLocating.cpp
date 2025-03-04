#include "Passes/KernelLocating.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/DataLayout.h"
#include <cstdint>
#include <fstream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{
    // Track loop hierarchy
    struct LoopHierarchy {
        int loopId;
        int parentId;
    };
    std::vector<LoopHierarchy> loopHierarchyInfo;
    set<Loop*> seenLoops;
    std::map<Loop*, int> loopToId;

    BasicBlock* getLoopBody(Loop* loop) {
        BasicBlock *Header = loop->getHeader(); // The loop header
        SmallVector<BasicBlock *, 8> LatchBlocks;
        loop->getLoopLatches(LatchBlocks); // Retrieve latch blocks

        BasicBlock *FirstBodyBlock = nullptr;

        for (BasicBlock *BB : loop->getBlocks()) {
            if (BB != Header && !llvm::is_contained(LatchBlocks, BB)) {
                FirstBodyBlock = BB;
                break;
            }
        }
        return FirstBodyBlock;
    }

    // Helper function to get all parent loops
    std::vector<Loop*> getParentLoops(Loop* innerLoop) {
        std::vector<Loop*> parents;
        Loop* current = innerLoop;
        while (current->getParentLoop() != nullptr) {
            parents.push_back(current->getParentLoop());
            current = current->getParentLoop();
        }
        // Reverse so outermost loop comes first
        std::reverse(parents.begin(), parents.end());
        return parents;
    }

    bool KernelLocating::runOnFunction(Function &F)
    {
        // Only process main function
        if (F.getName() != "main") {
            return false;
        }

        DominatorTree DT(F);
        LoopInfo LI(DT);

        if (LI.empty())
        {
            return false;
        }

        // First pass: Insert initialization at start of main
        auto firstBB = F.begin();
        auto firstInsertion = firstBB->getFirstInsertionPt();
        auto *firstInst = cast<Instruction>(firstInsertion);
        IRBuilder<> initBuilder(firstInst);
        initBuilder.CreateCall(LoopTraceInitialization);

        // Second pass: Insert all loop trace calls and collect hierarchy info
        for (auto BB = F.begin(); BB != F.end(); BB++)
        {
            for (BasicBlock::iterator BI = BB->begin(), BE = BB->end(); BI != BE; ++BI)
            {
                if (auto *CI = dyn_cast<CallInst>(BI))
                {
                    Function *calledFunc = CI->getCalledFunction();
                   
                    if (calledFunc == nullptr || !calledFunc->hasName())
                    {
                        continue;
                    }
                    if (calledFunc->getName() == "DASH_FFT" || 
                        calledFunc->getName() == "DASH_GEMM" || 
                        calledFunc->getName() == "NonKernelSplit")
                    {
                        auto *innerLoop = LI.getLoopFor(dyn_cast<BasicBlock>(BB));
                        if (innerLoop != nullptr) {
                            // Get all parent loops
                            std::vector<Loop*> parentLoops = getParentLoops(innerLoop);
                            
                            // Add instrumentation for parent loops first
                            Loop* lastParent = nullptr;
                            for (Loop* parentLoop : parentLoops) {
                                if (seenLoops.find(parentLoop) == seenLoops.end()) {
                                    seenLoops.insert(parentLoop);
                                    int loopID = seenLoops.size();
                                    loopToId[parentLoop] = loopID;

                                    // Record hierarchy
                                    LoopHierarchy hierarchy;
                                    hierarchy.loopId = loopID;
                                    hierarchy.parentId = lastParent ? loopToId[lastParent] : 0;
                                    loopHierarchyInfo.push_back(hierarchy);

                                    std::vector<Value *> args;
                                    Value *StageValue = ConstantInt::get(
                                        Type::getInt64Ty(BB->getContext()), 
                                        loopID
                                    );
                                    args.push_back(StageValue);
                                    
                                    IRBuilder<> Builder(&getLoopBody(parentLoop)->front());
                                    Builder.CreateCall(LoopTrace, args);
                                }
                                lastParent = parentLoop;
                            }

                            // Add instrumentation for innermost loop
                            if (seenLoops.find(innerLoop) == seenLoops.end()) {
                                seenLoops.insert(innerLoop);
                                int loopID = seenLoops.size();
                                loopToId[innerLoop] = loopID;

                                // Record hierarchy
                                LoopHierarchy hierarchy;
                                hierarchy.loopId = loopID;
                                hierarchy.parentId = lastParent ? loopToId[lastParent] : 0;
                                loopHierarchyInfo.push_back(hierarchy);

                                std::vector<Value *> args;
                                Value *StageValue = ConstantInt::get(
                                    Type::getInt64Ty(BB->getContext()), 
                                    loopID
                                );
                                args.push_back(StageValue);
                                // Place LoopTrace at the start of the loop body
                                IRBuilder<> Builder(&getLoopBody(innerLoop)->front());
                                Builder.CreateCall(LoopTrace, args);
                            }
                        }
                    }
                }
            }
        }

        // Third pass: Insert destruction call with complete hierarchy info
        // Find return instruction
        for (auto BB = F.begin(); BB != F.end(); BB++)
        {
            if (auto retInst = dyn_cast<ReturnInst>(BB->getTerminator()))
            {
                // Create array of loop hierarchy info
                std::vector<Constant*> hierarchyArgs;
                for (const auto& info : loopHierarchyInfo) {
                    hierarchyArgs.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()), info.loopId));
                    hierarchyArgs.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()), info.parentId));
                }
                
                // Create array constant
                ArrayType* arrayType = ArrayType::get(Type::getInt32Ty(F.getContext()), hierarchyArgs.size());
                Constant* arrayConstant = ConstantArray::get(arrayType, hierarchyArgs);
                
                // Create global variable for the array
                GlobalVariable* hierarchyArray = new GlobalVariable(
                    *F.getParent(),
                    arrayType,
                    true,
                    GlobalValue::PrivateLinkage,
                    arrayConstant,
                    "loopHierarchy"
                );

                IRBuilder<> lastBuilder(retInst);
                
                // Cast array to int32* before passing to function
                Value* castedArray = lastBuilder.CreateBitCast(hierarchyArray, 
                    Type::getInt32PtrTy(F.getContext()), 
                    "castedHierarchy");
                
                // Pass array and size to LoopTraceDestroy
                std::vector<Value*> args;
                args.push_back(castedArray);
                args.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()), hierarchyArgs.size()));
                lastBuilder.CreateCall(LoopTraceDestroy, args);
                break;
            }
        }

        return true;
    }

    bool KernelLocating::doInitialization(Module &M)
    {
        LoopTrace = cast<Function>(M.getOrInsertFunction("LoopTrace", 
            Type::getVoidTy(M.getContext()), 
            Type::getInt64Ty(M.getContext())).getCallee());

        LoopTraceInitialization = cast<Function>(M.getOrInsertFunction("LoopTraceInitialization", 
            Type::getVoidTy(M.getContext())).getCallee());

        // Fix: Change parameter type to match array pointer type
        LoopTraceDestroy = cast<Function>(M.getOrInsertFunction("LoopTraceDestroy",
            Type::getVoidTy(M.getContext()),
            Type::getInt32PtrTy(M.getContext()),  // Changed from arrayType to int32*
            Type::getInt32Ty(M.getContext())).getCallee());

        return false;
    }

    void KernelLocating::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char KernelLocating::ID = 1;
    static RegisterPass<KernelLocating> Y("KernelLocating", "locate the kernel related code", true, false);
} // namespace DashTracer::Passes