#include "Passes/SwapAPIs.h"
#include "AtlasUtil/Annotate.h"
#include "Passes/Annotate.h"
#include "Passes/CommandArgs.h"
#include "Passes/Functions.h"
#include "Passes/TraceMemIO.h"
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
#include <sys/prctl.h>
#include <unistd.h>

using namespace llvm;
using namespace std;
cl::opt<std::string> dummyBitcode("dm", cl::desc("Specify filename for dummy bitcode"), cl::value_desc("dummy bitcode"));

set<int64_t> startKernelIndex;
map<int64_t, int64_t> EndKernelIndexToCounter;

AllocaInst *cond_alloca;
AllocaInst *mutex_alloca;
AllocaInst *threads_done;

Module *dummyModule;
int tripCounter = 0;
bool function_is_swappable(std::string &funcName)
{
    return funcName == "DASH_ZIP" || funcName == "DASH_FFT" || funcName == "DASH_GEMM" || funcName == "DASH_FIR" || funcName == "DASH_SpectralOpening" || funcName == "DASH_CIC";
}

bool DashTracer::Passes::SwapAPIs::runOnFunction(Function &F)
{
    bool modified = false;
    Module *module = F.getParent();

    LLVMContext dmcontext;
    SMDiagnostic smerror;
    unique_ptr<Module> sourceBitcode;
    try
    {
        sourceBitcode = parseIRFile(dummyBitcode, smerror, dmcontext);
    }
    catch (exception &e)
    {
        return modified;
    }

    dummyModule = sourceBitcode.get();

    Function *cedr_dispatch;
    Function *mutex_lock;
    Function *mutex_unlock;
    Function *cond_init;
    Function *mutex_init;
    Function *cond_wait;
    StructType *cond_type = nullptr;
    StructType *mutex_type;
    StructType *condattr_type;
    StructType *mutexattr_type;

    cedr_dispatch = dummyModule->getFunction("enqueue_kernel");
    if (!cedr_dispatch)
    {
        errs() << "Unable to find the cedr dispatch function!\n";
        return modified;
    }
    mutex_lock = dummyModule->getFunction("pthread_mutex_lock");
    if (!mutex_lock)
    {
        errs() << "Unable to find the mutex_lock function!\n";
        return modified;
    }
    mutex_unlock = dummyModule->getFunction("pthread_mutex_unlock");
    if (!mutex_unlock)
    {
        errs() << "Unable to find the mutex_unlock function!\n";
        return modified;
    }
    cond_init = dummyModule->getFunction("pthread_cond_init");
    if (!cond_init)
    {
        errs() << "Unable to find the cond_init function!\n";
        return modified;
    }

    mutex_init = dummyModule->getFunction("pthread_mutex_init");
    if (!mutex_init)
    {
        errs() << "Unable to find the mutex_init function!\n";
        return modified;
    }
    cond_wait = dummyModule->getFunction("pthread_cond_wait");
    if (!cond_wait)
    {
        errs() << "Unable to find the cond_wait function!\n";
        return modified;
    }

    for (auto *st : module->getIdentifiedStructTypes())
    {
        if (st->getName() == "union.pthread_cond_t")
        {
            // errs() << "found cond_t \n";
            cond_type = st;
        }
        else if (st->getName() == "union.pthread_mutex_t")
        {
            // errs() << "found mutex_t \n";
            mutex_type = st;
        }
        else if (st->getName() == "union.pthread_condattr_t")
        {
            errs() << "found condattr_t \n";
            condattr_type = st;
        }
        else if (st->getName() == "union.pthread_mutexattr_t")
        {
            errs() << "found mutexattr_t \n";
            mutexattr_type = st;
        }
    }

    // For use in debugging through CLion by whitelisting CLion's PID as something
    // that can trace this process prctl(PR_SET_PTRACER, 17808, 0, 0, 0);

    // auto typeTest = &(*(cond_init->arg_begin() + 1)->getType()->getPointerElementType());
    // auto attr_type_ptr = PointerType::get(attr_type, module->getDataLayout().getProgramAddressSpace());

    PointerType *cond_type_ptr = nullptr;
    PointerType *mutex_type_ptr = nullptr;

    // StructType *condattr_type = nullptr;
    // StructType *mutexattr_type = nullptr;

    PointerType *condattr_type_ptr = nullptr;
    PointerType *mutexattr_type_ptr = nullptr;

    // else if (st->getName() == "union.condattr_t") {
    //   errs() << "found condattr_t \n";
    //   condattr_type = st;
    // } else if (st->getName() == "union.mutexattr_t") {
    //   errs() << "found mutexattr_t \n";
    //   mutexattr_type = st;
    // }

    if (!cond_type)
    {
        errs() << "I was unable to locate the StructType* associated with "
                  "cond_type!\n";
    }
    else
    {
        cond_type_ptr = PointerType::get(cond_type, module->getDataLayout().getProgramAddressSpace());
    }

    if (!condattr_type)
    {
        errs() << "I was unable to locate the StructType* associated with "
                  "condattr_type!\n";
    }
    else
    {
        condattr_type_ptr = PointerType::get(condattr_type, module->getDataLayout().getProgramAddressSpace());
    }

    if (!mutex_type)
    {
        errs() << "I was unable to locate the StructType* associated with "
                  "mutex_type!\n";
    }
    else
    {
        mutex_type_ptr = PointerType::get(mutex_type, module->getDataLayout().getProgramAddressSpace());
    }

    // if (!mutexattr_type)
    // {
    //     errs() << "I was unable to locate the StructType* associated with "
    //               "mutexattr_type!\n";
    // }
    // else
    // {
    //     mutexattr_type_ptr = PointerType::get(mutexattr_type, module->getDataLayout().getProgramAddressSpace());
    // }

    LLVMContext &context = module->getContext();
    // auto function_name = F.getName();
    // if (function_name != "main")
    // {
    //     errs() << "name:" << function_name << "\n";
    //     return modified;
    // }

    // for (auto &mi : *module)
    // {
    //     for (auto fi = mi.begin(); fi != mi.end(); fi++)
    //     {
    //         auto *bb = cast<BasicBlock>(fi);
    //         int64_t id = GetBlockID(bb);
    //         if (startKernelIndex.find(id) != startKernelIndex.end())
    //         {

    //             for (auto bi = fi->begin(); bi != fi->end(); bi++)
    //             {
    //                 if (auto *CI = dyn_cast<CallInst>(bi))
    //                 {
    //                   // condition init
    //                   IRBuilder<> builder(CI);
    //             cond_alloca = builder.CreateAlloca(cond_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");
    //             // ConstantPointerNull *null_attrs = ConstantPointerNull::get(condattr_type_ptr);
    //             ConstantPointerNull *null_attrs = ConstantPointerNull::get(condattr_type_ptr);

    //             SmallVector<Value *, 8> cond_init_args;
    //             cond_init_args.push_back(cond_alloca);
    //             cond_init_args.push_back(null_attrs);

    //             CallInst *cond_init_call = builder.CreateCall(cond_init, cond_init_args);
    //             cond_init_call->setCallingConv(cond_init->getCallingConv());

    //             // mutex init
    //             mutex_alloca = builder.CreateAlloca(mutex_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");

    //             null_attrs = ConstantPointerNull::get(mutexattr_type_ptr);

    //             SmallVector<Value *, 8> mutex_init_args;
    //             mutex_init_args.push_back(mutex_alloca);
    //             mutex_init_args.push_back(null_attrs);

    //             CallInst *mutex_init_call = builder.CreateCall(mutex_init, mutex_init_args);
    //             mutex_init_call->setCallingConv(mutex_init->getCallingConv());

    //             // thread done
    //             threads_done = builder.CreateAlloca(Type::getInt32Ty(context));
    //             StoreInst *threads_done_store = builder.CreateStore(builder.getInt32(0), threads_done);
    //             // mutex lock
    //             CallInst *mutex_lock_call = builder.CreateCall(mutex_lock, mutex_alloca);
    //             mutex_lock_call->setCallingConv(mutex_lock->getCallingConv());
    //             errs()<<"bb:"<<*bb<<"\n";
    //             break;

    //                 }
    //             }
    //         }
    //     }
    // }

    while (true)
    {

        Instruction *tobeSwaped;
        bool detectedSwap = false;
        std::string funcName;

        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;)
        {
            Instruction *inst = &*I;
            I++;

            if (auto *CI = dyn_cast<CallInst>(inst))
            {
                Function *calledFunc = CI->getCalledFunction();

                if (calledFunc)
                {
                    funcName = calledFunc->getName();
                    if (function_is_swappable(funcName))
                    {
                        detectedSwap = true;
                        tobeSwaped = CI;
                        auto Testname = inst->getParent()->getParent()->getName();
                        if (Testname != "main")
                        {
                            errs() << "name:" << Testname << "\n";
                            // return modified;
                        }
                        break;
                    }
                }
            }
        }

        if (!detectedSwap)
        {
            break;
        }
        else
        {
          tripCounter++;
            errs() << "one function found \n";

            // CurrentKernelIndexInsideStage++;
            // liangliang: thread variable initialization, lock

            // inserting before CI
            IRBuilder<> builder(tobeSwaped);
            // errs() << "bb:" << *(tobeSwaped->getParent()) << "\n";

            // std::string Str;
            // raw_string_ostream OS(Str);
            // tobeSwaped->getParent()->printAsOperand(OS, false);
            // tobeSwaped->getParent()->printAsOperand(errs(), false);
            // errs() << "name" << tobeSwaped->getParent()->getName() << "\n";
            // idtest = OS.str();
            int64_t idtest = GetBlockID(tobeSwaped->getParent());
            // if ( startKernelIndex.find(CurrentKernelIndexInsideStage) !=startKernelIndex.end() ) {
            // if (startKernelIndex.find(idtest) != startKernelIndex.end())
            if (tripCounter ==1 )
            {
                // condition init
                cond_alloca = builder.CreateAlloca(cond_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");
                // // ConstantPointerNull *null_attrs = ConstantPointerNull::get(condattr_type_ptr);
                // ConstantPointerNull *null_attrs = ConstantPointerNull::get(condattr_type_ptr);

                // SmallVector<Value *, 8> cond_init_args;
                // cond_init_args.push_back(cond_alloca);
                // cond_init_args.push_back(null_attrs);

                // CallInst *cond_init_call = builder.CreateCall(cond_init, cond_init_args);
                // cond_init_call->setCallingConv(cond_init->getCallingConv());

                // // mutex init
                // mutex_alloca = builder.CreateAlloca(mutex_type, module->getDataLayout().getAllocaAddrSpace(), nullptr, "");

                // null_attrs = ConstantPointerNull::get(mutexattr_type_ptr);

                // SmallVector<Value *, 8> mutex_init_args;
                // mutex_init_args.push_back(mutex_alloca);
                // mutex_init_args.push_back(null_attrs);

                // CallInst *mutex_init_call = builder.CreateCall(mutex_init, mutex_init_args);
                // mutex_init_call->setCallingConv(mutex_init->getCallingConv());

                // // thread done
                // threads_done = builder.CreateAlloca(Type::getInt32Ty(context));
                // errs() << "threads_done:" << *threads_done << "\n";
                // StoreInst *threads_done_store = builder.CreateStore(builder.getInt32(0), threads_done);
                // errs() << "threads_done_store:" << *threads_done_store << "\n";

                // // mutex lock

                // CallInst *mutex_lock_call = builder.CreateCall(mutex_lock, mutex_alloca);
                // mutex_lock_call->setCallingConv(mutex_lock->getCallingConv());
            }

            // // swap the functions

            // // -------- 3. Add in the call to enqueue_kernel (or whatever the function name is) ---------//

            // // Add the name of the kernel being called as the first argument
            // SmallVector<Value *, 16> kernel_args;
            // StringRef myStr = funcName;
            // Constant *kernNameConst = builder.CreateGlobalStringPtr(myStr, ".dash.kern_" + std::to_string(kern_instance_ctr));
            // kernel_args.push_back(kernNameConst);

            // // Propagate the rest of the arguments from the original call
            // auto *tobeSwapedCall = dyn_cast<CallInst>(tobeSwaped);
            // for (const auto &arg : tobeSwapedCall->args())
            // {
            //     const Use &use = arg;
            //     Value *val = use.get();
            //     // To get "pointers" to the values that were originally passed in, we just make a new Alloca here
            //     // And then we store the current value in that memory address
            //     // Thinking through cases where the original arg was either a scalar or a pointer,
            //     // I can't think of any edge cases where this would go wrong (?)
            //     AllocaInst *pointerAI = builder.CreateAlloca(val->getType());
            //     StoreInst *pointerStore = builder.CreateStore(val, pointerAI);

            //     kernel_args.push_back(pointerAI);
            // }
            // kernel_args.push_back(cond_alloca);
            // kernel_args.push_back(mutex_alloca);
            // kernel_args.push_back(threads_done);

            // CallInst *ReplacementCI = builder.CreateCall(cedr_dispatch, kernel_args);
            // ReplacementCI->setCallingConv(cedr_dispatch->getCallingConv());

            // // liangliang: while loop con wait and unlock, todo change it to be
            // // the tail of kernel stage
            // // if (EndKernelIndexToCounter.find(CurrentKernelIndexInsideStage) !=EndKernelIndexToCounter.end()) {

            // if (EndKernelIndexToCounter.find(idtest) != EndKernelIndexToCounter.end())
            // {
            //     // split the old basic block

            //     BasicBlock *parent = ReplacementCI->getParent();

            //     BasicBlock *newBB = parent->splitBasicBlock(ReplacementCI->getNextNode(), "splitBB");
            //     if (newBB->getInstList().size() > 1)
            //     {
            //         newBB->splitBasicBlock(newBB->front().getNextNode());
            //     }
            //     // errs() << "module" << *module;
            //     //   errs()<<"ReplacementCI"<<*ReplacementCI<<"\n";
            //     //   errs()<<"newBB"<<*newBB;

            //     BranchInst *parentBR = dyn_cast<BranchInst>(parent->getTerminator());

            //     BasicBlock *cmp = BasicBlock::Create(context, "cmp", &F);
            //     BasicBlock *condWaitingBB = BasicBlock::Create(context, "condWait", &F);
            //     BasicBlock *mutexUnlockBB = BasicBlock::Create(context, "mutexUnlock", &F);

            //     parentBR->setSuccessor(0, cmp);

            //     builder.SetInsertPoint(cmp);

            //     LoadInst *load_counter = builder.CreateLoad(threads_done);

            //     // to do 2 need to be changed to be the same as the counter
            //     Value *notEqual2 = builder.CreateICmpNE(load_counter, builder.getInt32(EndKernelIndexToCounter[idtest]), "tmp");
            //     builder.CreateCondBr(notEqual2, condWaitingBB, mutexUnlockBB);

            //     // waiting bb
            //     builder.SetInsertPoint(condWaitingBB);
            //     SmallVector<Value *, 8> cond_wait_args;
            //     cond_wait_args.push_back(cond_alloca);
            //     cond_wait_args.push_back(mutex_alloca);

            //     CallInst *CondWait = builder.CreateCall(cond_wait, cond_wait_args);
            //     CondWait->setCallingConv(cond_wait->getCallingConv());
            //     builder.CreateBr(cmp);

            //     // unlock bb
            //     builder.SetInsertPoint(mutexUnlockBB);
            //     // unlock the mutex in a new basic block
            //     CallInst *mutex_unlock_call = builder.CreateCall(mutex_unlock, mutex_alloca);
            //     mutex_unlock_call->setCallingConv(mutex_unlock->getCallingConv());
            //     builder.CreateBr(newBB);
            //     errs() << "module" << *module;
            //     // set inst counter to 0
            // }
            errs() << "bb:" << *(tobeSwaped->getParent()) << "\n";
            tobeSwaped->eraseFromParent();
            kern_instance_ctr++;
            modified = true;
        }
    }

    // Special case for the main function: inject a poison pill at the end
    //   if (F.getName() == "main") {
    //     SmallVector<Value *, 4> kernel_args;
    //     Instruction *lastInst = &(F.back().back());
    //     IRBuilder<> builder(lastInst);

    //     StringRef myStr = "POISON_PILL";
    //     Constant *kernNameConst = builder.CreateGlobalStringPtr(myStr, ".dash.poison_pill");
    //     kernel_args.push_back(kernNameConst);

    //     CallInst *ReplacementCI = builder.CreateCall(cedr_dispatch, kernel_args);
    //     ReplacementCI->setCallingConv(cedr_dispatch->getCallingConv());
    //     modified = true;
    //   }
    modified = true;
    return modified;
}

bool DashTracer::Passes::SwapAPIs::doInitialization(Module &M)
{
    bool modified = false;
    for (Module::iterator F = M.begin(), E = M.end(); F != E;)
    {
        Function *func = &*F;
        F++;
        if (func->getName() == "dummy_function")
        {
            func->eraseFromParent();
            modified = true;
        }
    }

    startKernelIndex.clear();
    EndKernelIndexToCounter.clear();
    nlohmann::json j;
    std::ifstream inputStream(DAGBBIOFile);
    inputStream >> j;
    inputStream.close();
    nlohmann::json mapping = j["EndKernelIndexToCounter"];
    EndKernelIndexToCounter = mapping.get<map<int64_t, int64_t>>();
    nlohmann::json setting = j["startKernelIndex"];
    startKernelIndex = setting.get<set<int64_t>>();

    return modified;
}

void DashTracer::Passes::SwapAPIs::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.addRequired<DashTracer::Passes::EncodedAnnotate>();
    AU.setPreservesCFG();
}

char DashTracer::Passes::SwapAPIs::ID = 0;
static RegisterPass<DashTracer::Passes::SwapAPIs> Z("SwapAPIs",
                                                    "Find instances of DASH API calls throughout a given set of LLVM modules "
                                                    "and swap them for CEDR invocations",
                                                    false, false);
