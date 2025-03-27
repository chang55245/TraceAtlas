#include "Backend/LoopTrace.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>

#define MAX_LOOP_COUNT 1000  // Adjust size as needed

struct LoopCount {
    int loopId;
    int count;
    int parentId;  // Will be set from hierarchy info
    bool isActive;  // To track which entries are in use
};

static LoopCount loop_array[MAX_LOOP_COUNT] = {0};
static int next_free_index = 0;

extern "C" {
    void LoopTrace(int loopID)
    {
        // Find existing entry
        for (int i = 0; i < next_free_index; i++) {
            if (loop_array[i].isActive && loop_array[i].loopId == loopID) {
                loop_array[i].count++;
                return;
            }
        }

        // Create new entry
        if (next_free_index < MAX_LOOP_COUNT) {
            loop_array[next_free_index].loopId = loopID;
            loop_array[next_free_index].count = 1;
            loop_array[next_free_index].parentId = 0;
            loop_array[next_free_index].isActive = true;
            next_free_index++;
        }
    }

    void LoopTraceInitialization()
    {
        // Reset the array
        memset(loop_array, 0, sizeof(loop_array));
        next_free_index = 0;
    }

    void LoopTraceDestroy(int* hierarchyInfo, int hierarchySize)
    {
        try {
            // Update parent IDs from hierarchy info
            for (int i = 0; i < hierarchySize; i += 2) {
                int loopId = hierarchyInfo[i];
                int parentId = hierarchyInfo[i + 1];
                
                // Find and update the corresponding loop count
                for (int j = 0; j < next_free_index; j++) {
                    if (loop_array[j].isActive && loop_array[j].loopId == loopId) {
                        loop_array[j].parentId = parentId;
                        break;
                    }
                }
            }

            // Recalculate counts by dividing by parent counts
            for (int i = 0; i < next_free_index; i++) {
                if (!loop_array[i].isActive) continue;
                if (loop_array[i].parentId != 0) {
                    // Find parent's count
                    for (int j = 0; j < next_free_index; j++) {
                        if (loop_array[j].isActive && 
                            loop_array[j].loopId == loop_array[i].parentId) {
                            if (loop_array[j].count > 0) {
                                loop_array[i].count = loop_array[i].count / loop_array[j].count;
                            }
                            break;
                        }
                    }
                }
            }

            const char* LoopTraceEnvFile = getenv("LoopTrace_FILE");
            const char* filename = LoopTraceEnvFile ? LoopTraceEnvFile : "LoopTraceFile.json";

            FILE* file = fopen(filename, "w");
            if (!file) {
                fprintf(stderr, "Error: Failed to open file %s\n", filename);
                return;
            }

            fprintf(file, "{\n    \"loopIteration\": [\n");
            bool first = true;
            for (int i = 0; i < next_free_index; i++) {
                if (!loop_array[i].isActive) continue;
                if (!first) fprintf(file, ",\n");
                fprintf(file, "        {\n");
                fprintf(file, "            \"loopId\": %d,\n", loop_array[i].loopId);
                fprintf(file, "            \"count\": %d,\n", loop_array[i].count);
                fprintf(file, "            \"parentId\": %d\n", loop_array[i].parentId);
                fprintf(file, "        }");
                first = false;
            }
            
            fprintf(file, "\n    ]\n}\n");
            fclose(file);

            // Reset the array
            LoopTraceInitialization();
        } catch (const std::exception& e) {
            std::cerr << "Error in LoopTraceDestroy: " << e.what() << std::endl;
        }
    }
}