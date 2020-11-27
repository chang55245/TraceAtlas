#include "AtlasUtil/Traces.h"
#include <algorithm>
#include <fstream>
#include <llvm/Support/CommandLine.h>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <iostream>

using namespace llvm;
using namespace std;

cl::opt<std::string> InputFilename("i", cl::desc("Specify input trace"), cl::value_desc("trace filename"), cl::Required);
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output json"), cl::value_desc("output filename"), cl::Required);
llvm::cl::opt<bool> noBar("nb", llvm::cl::desc("No progress bar"), llvm::cl::value_desc("No progress bar"));

namespace myns {
    struct labelGroup_s {
        string kernelLabel;
        set<int64_t> blocks;
        uint64_t instanceNum;
        uint64_t globalUID;
    };
    typedef struct labelGroup_s labelGroup;
    void to_json(nlohmann::json &j, const labelGroup &l) {
        j = nlohmann::json{{"kernelLabel", l.kernelLabel}, {"blocks", l.blocks}, {"instanceNum", l.instanceNum}, {"globalUID", l.globalUID}};
    }
}

static int64_t GLOBAL_UID = 0;

map<int64_t, set<string>> labels;
//map<string, vector<int64_t>> labelToBBs;
//vector<pair<string, vector<int64_t>>> labelsAndBBVecs;
set<string> currentLabels;
bool inKernel = false;
vector<myns::labelGroup> labelsAndBBVecs;
map<string, uint64_t> kernelInstanceCounter;

vector<int64_t> blockStack;

bool structEq(myns::labelGroup &gp1, myns::labelGroup &gp2) {
    if (gp1.kernelLabel != gp2.kernelLabel) {
        return false;
    }
    if (gp1.blocks.size() != gp2.blocks.size()) {
        return false;
    }
    if (gp1.blocks != gp2.blocks) {
        return false;
    }
    return true;
}

void Process(string &key, string &value)
{
    if (key == "BBEnter") {
        blockStack.push_back(stol(value, nullptr, 0));
    }
    if (key == "BBExit")
    {
        int64_t block = stol(value, nullptr, 0);
        labels[block].insert(currentLabels.begin(), currentLabels.end());
        blockStack.pop_back();

        if (!currentLabels.empty()) {
            labelsAndBBVecs.back().blocks.insert(block);
        }
    }
    else if (key == "KernelEnter")
    {
        currentLabels.insert(value);
        if (kernelInstanceCounter.find(value) == kernelInstanceCounter.end()) {
            kernelInstanceCounter[value] = 1;
        } else {
            kernelInstanceCounter[value]++;
        }

        labelsAndBBVecs.emplace_back();
        labelsAndBBVecs.back().kernelLabel = value;
        labelsAndBBVecs.back().blocks = {};
        labelsAndBBVecs.back().instanceNum = kernelInstanceCounter[value];
        labelsAndBBVecs.back().globalUID = GLOBAL_UID++;
    }
    else if (key == "KernelExit")
    {
        // Note: we need to capture the current basic block as part of the kernel potentially as well.
        // The top element of the basic block stack should correspond to the block that this kernel exit resides in (?)
        labelsAndBBVecs.back().blocks.insert(blockStack.back());
        currentLabels.erase(value);
        for (auto idx = 0; idx < labelsAndBBVecs.size()-1; idx++) {
            if (structEq(labelsAndBBVecs.at(idx), labelsAndBBVecs.back())) {
                //cout << "I found a duplicate struct and I'm dropping it" << endl;
                labelsAndBBVecs.pop_back();
                kernelInstanceCounter[value]--;
                GLOBAL_UID--;
            }
        }
    }
}

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    ProcessTrace(InputFilename, Process, "Generating JR", noBar);
    std::ofstream file;
    nlohmann::json jOut;
    nlohmann::json jOut2;
    jOut2["kernels"] = nlohmann::json::array();
    for (const auto &pair : labels)
    {
        string label = std::to_string(pair.first);
        jOut[label] = pair.second;
    }
    // Fixup labelsAndBBVecs by looking for missing intermediate blocks that were never entered and thus skipped from tracing
    for (auto &struc : labelsAndBBVecs) {
        auto &blks = struc.blocks;
        if (blks.empty()) {
            continue;
        }
        const auto minBlk = *struc.blocks.begin();
        const auto maxBlk = *struc.blocks.rbegin();
        for (auto blkNum = minBlk; blkNum < maxBlk; blkNum++) {
            // If we have blkNum and blkNum + 2 but not blkNum + 1, add it
            if (blks.find(blkNum) != blks.end() && blks.find(blkNum+2) != blks.end() && blks.find(blkNum+1) == blks.end()) {
                cout << "Inserting a block that was missing between " << blkNum << " and " << blkNum + 2 << endl;
                blks.insert(blkNum+1);
            }
        }
    }
    for (const auto &struc : labelsAndBBVecs) {
        jOut2["kernels"].push_back(nlohmann::json(struc));
    }
//    for (const auto &pair : labelToBBs) {
//        cout << "Key: " << pair.first << endl;
//        cout << "BBs: {";
//        for (const auto &bb : pair.second) {
//            cout << bb << ", ";
//        }
//        cout << "}" << endl;
//    }
//    for (const auto &elem : labelsAndBBVecs) {
//        cout << "Kernel: " << elem.kernelLabel << endl;
//        cout << "Instance num: " << elem.instanceNum << endl;
//        cout << "Blocks: {";
//        for (auto &blk : elem.blocks) {
//            cout << blk << ", ";
//        }
//        cout << "}" << endl;
//        cout << endl;
//    };
    file.open(OutputFilename);
    //file << jOut.dump(4);
    file << jOut2.dump(4);
    file.close();
}