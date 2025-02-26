#include "Backend/LoopTrace.h"
#include <assert.h>
#include <map>
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
map<int, int> loopIteration;


extern "C"
{
    void LoopTrace(int loopID)
    {
        if (loopIteration.find(loopID) != loopIteration.end())
        {
            loopIteration[loopID]++;
        }
        else
        {
            loopIteration.insert(make_pair(loopID, 1));
        }
    }
    void LoopTraceInitialization()
    {
        loopIteration.clear();
    }
    void LoopTraceDestroy()
    {
        nlohmann::json jOut;
        
        for (const auto& [key, value] : loopIteration) {
            jOut["loopIteration"].push_back({key, value});
        }


        const char* LoopTraceEnvFile = getenv("LoopTrace_FILE");
        std::string filename;
        if (LoopTraceEnvFile == nullptr) {
            filename = "LoopTraceFile.json";
        } else {
            filename = LoopTraceEnvFile;
        }

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open file " << filename << std::endl;
            return;
        }
        file << jOut.dump() << std::endl;
    }
}