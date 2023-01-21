#include "Passes/OmpNonkernelParallel.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
#include "llvm/IR/DataLayout.h"
#include <fstream>
#include <llvm-9/llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"



using namespace llvm;
using namespace std;

namespace DashTracer::Passes
{

    bool OmpNkParallel::runOnFunction(Function &F)
    {
        DominatorTree DT(F);
        LoopInfo LI(DT);
        set<Loop *> loopSet;
        int loop_counter = 0;
        for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
        {
            auto *block = cast<BasicBlock>(BB);

            // for (BasicBlock::iterator BI = block->begin(), BE = block->end(); BI != BE; ++BI)
            {

                auto *loopTest = LI.getLoopFor(block);
                if (loopTest != nullptr && loopSet.find(loopTest) == loopSet.end())
                {
                    loop_counter++;
                    loopSet.insert(loopTest);
                    if (loop_counter == 1)
                    {
                        errs() << "got the first loop \n";
                    }
                    // a loop should be put in subsections
                    if (loop_counter < 6)
                    {
                        errs() << "loop id:" << loop_counter << "\n";
                    }
                }
            }
        }

        // using InsertPointTy = OpenMPIRBuilder::InsertPointTy;
        // using BodyGenCallbackTy = llvm::OpenMPIRBuilder::StorableBodyGenCallbackTy;
        // OpenMPIRBuilder OMPBuilder(*M);
        // OMPBuilder.initialize();
        // F->setName("func");
        // IRBuilder<> Builder(BB);

        // BasicBlock *EnterBB = BasicBlock::Create(Ctx, "sections.enter", F);
        // Builder.CreateBr(EnterBB);
        // Builder.SetInsertPoint(EnterBB);
        // OpenMPIRBuilder::LocationDescription Loc({Builder.saveIP(), DL});

        // llvm::SmallVector<BodyGenCallbackTy, 4> SectionCBVector;
        // llvm::SmallVector<BasicBlock *, 4> CaseBBs;

        // auto FiniCB = [&](InsertPointTy IP) {};
        // auto SectionCB = [&](InsertPointTy AllocaIP, InsertPointTy CodeGenIP) {};
        // SectionCBVector.push_back(SectionCB);

        // auto PrivCB = [](InsertPointTy AllocaIP, InsertPointTy CodeGenIP,
        //                  llvm::Value &, llvm::Value &Val,
        //                  llvm::Value *&ReplVal)
        // { return CodeGenIP; };
        // IRBuilder<>::InsertPoint AllocaIP(&F->getEntryBlock(),
        //                                   F->getEntryBlock().getFirstInsertionPt());
        // Builder.restoreIP(OMPBuilder.createSections(Loc, AllocaIP, SectionCBVector,
        //                                             PrivCB, FiniCB, false, false));
        // Builder.CreateRetVoid(); // Required at the end of the function

        return true;
    }

    bool OmpNkParallel::doInitialization(Module &M)
    {
        return false;
    }

    void OmpNkParallel::getAnalysisUsage(AnalysisUsage &AU) const
    {
        AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
        AU.setPreservesCFG();
    }
    char OmpNkParallel::ID = 1;
    static RegisterPass<OmpNkParallel> Y("OmpNkParallel", "generate test tranformation", true, false);
} // namespace DashTracer::Passes