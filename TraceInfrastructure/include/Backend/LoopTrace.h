#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <map>

#ifdef __cplusplus
extern "C"
{
#endif
    void LoopTrace(uint64_t loopID, void* loopIterationVoid);
    void* LoopTraceInitialization();
    void LoopTraceDestroy(void* loopIterationVoid);
#ifdef __cplusplus
}
#endif