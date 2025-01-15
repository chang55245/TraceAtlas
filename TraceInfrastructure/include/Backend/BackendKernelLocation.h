#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
    void CallStartTrace(uint64_t callID);
    void CallEndTrace(uint64_t callID);
    void CallTraceInitialization();
    void CallTraceDestroy();

    // how to define a set of names? 
    // - pass by string
    // - pass by integer and keep a table
    // - pass by enum 
    // currently 0->DASH_FFT
    void KernelNameTrace(uint64_t nameID);
#ifdef __cplusplus
}
#endif