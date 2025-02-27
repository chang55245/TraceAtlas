#include "Backend/LoopTrace.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <iostream>

// Use C-style map implementation to avoid STL initialization issues
struct LoopCount {
    int loopId;
    int count;
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
        new_count->next = loop_counts;
        loop_counts = new_count;
    }

    void LoopTraceInitialization()
    {
        // Just ensure the list is empty
        while (loop_counts != nullptr) {
            LoopCount* temp = loop_counts;
            loop_counts = loop_counts->next;
            delete temp;
        }
    }

    void LoopTraceDestroy()
    {
        try {
            // Get filename
            const char* LoopTraceEnvFile = getenv("LoopTrace_FILE");
            std::string filename = LoopTraceEnvFile ? LoopTraceEnvFile : "LoopTraceFile.json";

            std::ofstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Failed to open file " << filename << std::endl;
                return;
            }

            // Write JSON manually
            file << "{\n    \"loopIteration\": [\n";
            
            // Write loop counts
            bool first = true;
            LoopCount* current = loop_counts;
            // {"loopIteration":[[1,10]]}
            while (current != nullptr) {
                if (!first) {
                    file << ",\n";
                }
                file << "            [" << current->loopId << ", " << current->count << "]";
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