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
map<uint64_t, uint64_t> loopIteration;

extern "C"
{
    void LoopTrace(uint64_t loopID)
    {
        if (loopIteration.find(loopID) != loopIteration.end())
        {
            loopIteration[loopID]++;
        }
        else
        {
            loopIteration[loopID] = 1;
        }
    }
    void LoopTraceInitialization()
    {
        loopIteration.clear();
    }
    void LoopTraceDestroy()
    {
        nlohmann::json jOut;
        jOut["loopIteration"] = loopIteration;
        std::ofstream file;

        const char *LoopTraceEnvFile = getenv("LoopTrace_FILE");
        if (LoopTraceEnvFile == nullptr)
        {
            LoopTraceEnvFile = &"LoopTraceFile.json"[0];
        }

        file.open(LoopTraceEnvFile);
        file << jOut << std::endl;
        file.close();

    }
}