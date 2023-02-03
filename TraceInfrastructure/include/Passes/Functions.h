#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <llvm/IR/Function.h>

using namespace llvm;

namespace DashTracer
{
    namespace Passes
    {
        extern Function *openFunc;
        extern Function *closeFunc;
        extern Function *BB_ID;
        extern Function *StoreDump;
        extern Function *DumpStoreValue;
        extern Function *LoadDump;
        extern Function *DumpLoadValue;
        extern Function *fullFunc;
        extern Function *fullAddrFunc;
        extern Function *MemCpyDump;

        extern Function *MainStart;
        extern Function *StartNonKernelStage;
        extern Function *EndNonKernelStage;

        extern Function *StartKernelStage;
        extern Function *EndKernelStage;
        extern Function *MiddleKernelStage;
        
        
        
        extern Function *SingleKernelStage;
        extern Function *LoopTrace;
        extern Function *LoopTraceInitialization;
        extern Function *LoopTraceDestroy;

        extern Function *CallStartTrace;
        extern Function *CallEndTrace;
        extern Function *CallTraceInitialization;
        extern Function *CallTraceDestroy;
        extern Function *KernelNameTrace;
     
    } // namespace Passes
} // namespace DashTracer

#endif