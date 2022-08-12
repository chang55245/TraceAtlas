#include "Backend/LoopTrace.h"
#include <assert.h>
#include <map>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using namespace std;
// Backend generate a map loopIteration<int,int>  [loopID, iterationCount]


extern "C"
{
    void LoopTrace(uint64_t loopID)
    {

    }
    void LoopTraceInitialization()
    {
        
    }
    void LoopTraceDestroy()
    {

    }
}