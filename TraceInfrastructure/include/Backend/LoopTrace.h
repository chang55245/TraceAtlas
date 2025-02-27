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
    void LoopTraceDestroy(int* hierarchyInfo, int hierarchySize);
#ifdef __cplusplus
}
#endif