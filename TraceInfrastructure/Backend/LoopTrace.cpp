#include "Backend/LoopTrace.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>

// Structure to represent loop hierarchy
struct LoopCount {
    int loopId;
    int count;
    int parentId;  // Will be set from hierarchy info
    struct LoopCount* next;
};

static LoopCount* loop_counts = nullptr;

extern "C" {
    void LoopTrace(int loopID)
    {
        // Find existing entry or create new one
        LoopCount* current = loop_counts;
        while (current != nullptr) {
            if (current->loopId == loopID) {
                current->count++;
                return;
            }
            current = current->next;
        }

        // Create new entry
        LoopCount* new_count = new LoopCount();
        new_count->loopId = loopID;
        new_count->count = 1;
        new_count->parentId = 0;  // Will be set from hierarchy info later
        new_count->next = loop_counts;
        loop_counts = new_count;
    }

    void LoopTraceInitialization()
    {
        while (loop_counts != nullptr) {
            LoopCount* temp = loop_counts;
            loop_counts = loop_counts->next;
            delete temp;
        }
    }

    void LoopTraceDestroy(int* hierarchyInfo, int hierarchySize)
    {
        try {
            // Update parent IDs from hierarchy info
            for (int i = 0; i < hierarchySize; i += 2) {
                int loopId = hierarchyInfo[i];
                int parentId = hierarchyInfo[i + 1];
                
                // Find and update the corresponding loop count
                LoopCount* current = loop_counts;
                while (current != nullptr) {
                    if (current->loopId == loopId) {
                        current->parentId = parentId;
                        break;
                    }
                    current = current->next;
                }
            }

            const char* LoopTraceEnvFile = getenv("LoopTrace_FILE");
            std::string filename = LoopTraceEnvFile ? LoopTraceEnvFile : "LoopTraceFile.json";

            std::ofstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Failed to open file " << filename << std::endl;
                return;
            }

            // Write JSON
            file << "{\n    \"loopIteration\": [\n";
            
            bool first = true;
            LoopCount* current = loop_counts;
            while (current != nullptr) {
                if (!first) file << ",\n";
                file << "        {\n";
                file << "            \"loopId\": " << current->loopId << ",\n";
                file << "            \"count\": " << current->count << ",\n";
                file << "            \"parentId\": " << current->parentId << "\n";
                file << "        }";
                first = false;
                current = current->next;
            }
            
            file << "\n    ]\n}\n";
            file.close();

            // Cleanup
            while (loop_counts != nullptr) {
                LoopCount* temp = loop_counts;
                loop_counts = loop_counts->next;
                delete temp;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in LoopTraceDestroy: " << e.what() << std::endl;
        }
    }
}