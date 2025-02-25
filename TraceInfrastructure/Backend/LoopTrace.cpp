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



extern "C"
{
    void LoopTrace(uint64_t loopID, void* loopIterationVoid)
    {
        fprintf(stderr, "LoopTrace called with loopID=%lu, ptr=%p\n", loopID, loopIterationVoid);
        
        if (loopIterationVoid == nullptr) {
            fprintf(stderr, "Error: loopIterationVoid is null in LoopTrace\n");
            return; // Early return to avoid segfault
        }
        
        fprintf(stderr, "About to cast loopIterationVoid to map\n");
        map<uint64_t, uint64_t>* loopIteration = static_cast<map<uint64_t, uint64_t>*>(loopIterationVoid);
        fprintf(stderr, "Cast successful, map address: %p\n", (void*)loopIteration);
        
        // Add a try-catch block to catch any potential issues
        try {
            fprintf(stderr, "Checking if loopID %lu exists in map\n", loopID);
            
            // Use a safer approach to update the map
            auto it = loopIteration->find(loopID);
            if (it != loopIteration->end()) {
                fprintf(stderr, "loopID %lu exists, incrementing from %lu\n", loopID, it->second);
                it->second++;
                fprintf(stderr, "New value: %lu\n", it->second);
            } else {
                fprintf(stderr, "loopID %lu does not exist, creating new entry\n", loopID);
                loopIteration->insert(std::make_pair(loopID, 1));
                fprintf(stderr, "Insertion complete\n");
            }
            
            fprintf(stderr, "LoopTrace completed successfully for loopID=%lu\n", loopID);
        } catch (const std::exception& e) {
            fprintf(stderr, "Exception in LoopTrace: %s\n", e.what());
        } catch (...) {
            fprintf(stderr, "Unknown exception in LoopTrace\n");
        }
    }
    
    void* LoopTraceInitialization()
    {
        try {
            map<uint64_t, uint64_t> *loopIteration = new map<uint64_t, uint64_t>;
            loopIteration->clear();
            fprintf(stderr, "LoopTraceInitialization: Created map at %p\n", (void*)loopIteration);
            return static_cast<void*>(loopIteration);
        } catch (const std::exception& e) {
            fprintf(stderr, "Exception in LoopTraceInitialization: %s\n", e.what());
            return nullptr;
        }
    }
    
    void LoopTraceDestroy(void* loopIterationVoid)
    {
        if (loopIterationVoid == nullptr) {
            fprintf(stderr, "Error: loopIterationVoid is null in LoopTraceDestroy\n");
            return;
        }
        
        try {
            map<uint64_t, uint64_t>* loopIteration = static_cast<map<uint64_t, uint64_t>*>(loopIterationVoid);
            fprintf(stderr, "LoopTraceDestroy: Processing map at %p\n", (void*)loopIteration);
            
            // Write to file using simple fprintf
            const char *LoopTraceEnvFile = getenv("LoopTrace_FILE");
            if (LoopTraceEnvFile == nullptr) {
                LoopTraceEnvFile = "LoopTraceFile.json";
            }
            
            fprintf(stderr, "Writing to file: %s\n", LoopTraceEnvFile);
            
            FILE* fp = fopen(LoopTraceEnvFile, "w");
            if (fp != nullptr) {
                fprintf(fp, "{\n  \"loopIteration\": {\n");
                
                // Manually write each entry
                bool first = true;
                for (const auto& pair : *loopIteration) {
                    if (!first) {
                        fprintf(fp, ",\n");
                    }
                    fprintf(fp, "    \"%lu\": %lu", pair.first, pair.second);
                    first = false;
                }
                
                fprintf(fp, "\n  }\n}\n");
                fclose(fp);
                fprintf(stderr, "File written successfully\n");
            } else {
                fprintf(stderr, "Failed to open file for writing\n");
            }
            
            delete loopIteration;
            fprintf(stderr, "Map deleted successfully\n");
        } catch (const std::exception& e) {
            fprintf(stderr, "Exception in LoopTraceDestroy: %s\n", e.what());
        } catch (...) {
            fprintf(stderr, "Unknown exception in LoopTraceDestroy\n");
        }
    }
}