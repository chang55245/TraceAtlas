#include "Backend/BackendKernelLocation.h"
#include <assert.h>
#include <cstdint>
#include <map>
#include <zlib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <utility>

using namespace std;
// Backend generate a map loopIteration<int,int>  [loopID, iterationCount]


extern "C"
{
    uint64_t currentCall = 0;

    // callID, inline function nameID
    map<uint64_t,uint64_t> inlineMap;
    void CallStartTrace(uint64_t callID)
    {
        currentCall = callID;
    }

    void CallEndTrace(uint64_t callID)
    {
        currentCall = 0;
    }
    void CallTraceInitialization()
    {
        currentCall = 0;
    }

    void KernelNameTrace(uint64_t nameID)
    {
        // currentCall==0 represents direct function call not using function pointer
        if (currentCall != 0U) {
            inlineMap[currentCall] = nameID;
        }
        
        currentCall = 0;
    }


    void CallTraceDestroy()
    {
        nlohmann::json jOut;
        jOut["inlineMap"] = inlineMap;
        std::ofstream file;

        const char *LoopTraceEnvFile = getenv("InlineMap_FILE");
        if (LoopTraceEnvFile == nullptr)
        {
            LoopTraceEnvFile = &"InlineMap.json"[0];
        }

        file.open(LoopTraceEnvFile);
        file << jOut << std::endl;
        file.close();

    }
}