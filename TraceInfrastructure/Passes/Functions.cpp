#include "Passes/Functions.h"
#include <llvm/IR/Function.h>
using namespace llvm;

namespace DashTracer::Passes
{
    Function *openFunc;
    Function *closeFunc;
    Function *BB_ID;
    Function *StoreDump;
    Function *DumpStoreValue;
    Function *LoadDump;
    Function *DumpLoadValue;
    Function *fullFunc;
    Function *fullAddrFunc;
    Function *MemCpyDump;


    Function *MainStart;
    Function *StartNonKernelStage;
    Function *EndNonKernelStage;

    Function *StartKernelStage;
    Function *EndKernelStage;
    Function *MiddleKernelStage;
    Function *SingleKernelStage;
    Function *LoopTrace;
    Function *LoopTraceInitialization;
    Function *LoopTraceDestroy;

    Function *CallEndTrace;
    Function *CallStartTrace;
    Function *CallTraceInitialization;
    Function *CallTraceDestroy;
    Function *KernelNameTrace;
} // namespace DashTracer::Passes