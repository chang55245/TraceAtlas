#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>

typedef struct {
    std::set<int> *kernel; // should be initialized at runtime
    int kernStart = -1; // so that we can't match with a blockID on the first pass
    std::set<int> *finalBlocks = new std::set<int>;
    std::set<int> *blocks = new std::set<int>;
} kernelBlock;

std::map<int, std::vector<int>> ExtractKernels(std::string sourceFile, std::vector<std::set<int>> kernels, bool newline);
