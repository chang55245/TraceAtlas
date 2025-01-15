#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void LoopTrace(uint64_t loopID);
    void LoopTraceInitialization();
    void LoopTraceDestroy();
#ifdef __cplusplus
}
#endif