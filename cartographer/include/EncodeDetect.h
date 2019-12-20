#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <set>

#define BLOCK_SIZE 16384

std::ifstream::pos_type filesize(std::string filename);
std::vector<std::set<int>> DetectKernels(std::string sourceFile, float thresh, int hotThreash, bool newline);
