#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <map>

#ifdef __cplusplus
extern "C"
{
#endif
    void LoopTrace(int loopID);
    void LoopTraceInitialization();
    void LoopTraceDestroy();
#ifdef __cplusplus
}
#endif