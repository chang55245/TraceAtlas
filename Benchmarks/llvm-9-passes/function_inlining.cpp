#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Use.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>
#include <system_error>

#include <llvm/Transforms/Utils/Cloning.h>

using namespace llvm;
using namespace std;

struct DynmFunctionInlining : public ModulePass
{
    static char ID;
    DynmFunctionInlining() : ModulePass(ID) { kern_instance_ctr = 0; }
    bool runOnModule(Module &M) override;
    // bool doInitialization(Loop *L, LPPassManager &LPM) override;

private:
    uint64_t kern_instance_ctr;
};


bool DynmFunctionInlining::runOnModule(Module &M)
{
    bool isModified = false;
    set <StringRef> inlinedFuncs;
    set <Instruction*> inlinedCalls;

    while (true)
    {
        StringRef inlineFuncName;
        bool detectedInline = false;

        // Iterate through all functions in the module to find a function to inline
        for (auto &F : M)
        {
            if (detectedInline) break;

            for (auto &BB : F)
            {
                if (detectedInline) break;

                for (auto &ii : BB)
                {
                    if (auto *CI = dyn_cast<CallInst>(&ii))
                    {
                        Function *fun = CI->getCalledFunction();
                        if (fun && (fun->getName() == "DASH_FFT" || fun->getName() == "DASH_GEMM"))
                        {
                            if (F.getName() != "main" && inlinedFuncs.find(F.getName()) == inlinedFuncs.end())
                            {
                                inlineFuncName = F.getName();
                                errs() <<"inline function: " <<inlineFuncName<<"\n";
                                detectedInline = true;
                                break;
                            }
                        }
                    }
                }
            } 
        }

        if (!detectedInline) break;

        // Iterate through all functions in the module to find a call to the function to inline
        
        // 0: call, 1: invoke
        int inline_type = -1;

        for (auto &F : M)
        {
            for (auto &BB : F)
            {

                for (auto &ii : BB)
                {
                    if (auto *CI = dyn_cast<CallInst>(&ii))
                    {
                        Function *fun = CI->getCalledFunction();
                        if (fun && fun->getName() == inlineFuncName)
                        {

                            errs() <<"inline instruction: " <<*CI<<"\n";
                            inlinedCalls.insert(CI);

                            inline_type = 0;
                        }
                    }
                    else if (auto *CI = dyn_cast<InvokeInst>(&ii))
                    {
                        Function *fun = CI->getCalledFunction();
                        if (fun && fun->getName() == inlineFuncName&& !fun->isDeclaration())
                        {

                            errs() <<"inline instruction: " <<*CI<<"\n";
                            inlinedCalls.insert(CI);

                            inline_type = 1;
                        }
                    }
                }
            }
        }

        for(auto inlineCall: inlinedCalls)
        {
            errs() << "Found\n";
            InlineFunctionInfo ifi = InlineFunctionInfo(NULL);
            if (auto *call = dyn_cast<CallInst>(inlineCall))
            {
                InlineFunction(call, ifi);
            }
            else if (auto *call = dyn_cast<InvokeInst>(inlineCall))
            {
                InlineFunction(call, ifi);
            }
            
            inlinedFuncs.insert(inlineFuncName);
            isModified = true;
        }
        inlinedCalls.clear();
    }



    return isModified;
}

char DynmFunctionInlining::ID = 0;
static RegisterPass<DynmFunctionInlining> Z("DynmFunctionInlining",
                                            "DynmFunctionInlining",
                                            false, false);
