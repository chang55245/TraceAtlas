#include "AtlasUtil/Annotate.h"
#include "AtlasUtil/Traces.h"
#include "llvm/ADT/SmallVector.h"
#include <algorithm>
#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <llvm/Analysis/DependenceAnalysis.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SourceMgr.h>
#include <map>
#include <nlohmann/json.hpp>
#include <queue>
#include <set>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <time.h>
#include <tuple>
#include <utility>
#include <vector>
#include <zlib.h>

#define LOADKEY 1
#define STOREKEY 2
using namespace llvm;
using namespace std;

cl::opt<std::string> InputFilename("t", cl::desc("Specify input trace"), cl::value_desc("trace filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify kernel json"), cl::value_desc("kernel filename"), cl::Required);

// node id -> set of bb
map<int, set<int>> kernelControlMap;

// (node id, load:0 or store:1) -> set of addresses
map<int,map<int,set<uint64_t>>> nodeAddrs;

void parsingKernelInfo(const string& KernelFilename)
{
    ifstream inputJson(KernelFilename);
    nlohmann::json j;
    inputJson >> j;
    inputJson.close();
    for (auto &[k, l] : j["Control"].items())
    {
        int index = stoul(k, nullptr, 0);
        nlohmann::json kernel = l["Blocks"];
        kernelControlMap[index] = kernel.get<set<int>>();
    }
}

void Process(string &key, string &value)
{
    static int currentNode = 0;
    if (key == "BBEnter")
    {
        int block = stoi(value, nullptr, 0);
        // get into another node
        if (kernelControlMap[currentNode].find(block)==kernelControlMap[currentNode].end()) {
            currentNode++;
        }

    }
    else if (key == "StoreAddress")
    {
        uint64_t address = stoul(value, nullptr, 0);
        nodeAddrs[currentNode][STOREKEY].insert(address);  
    }
    else if (key == "LoadAddress")
    {
        uint64_t address = stoul(value, nullptr, 0);
        nodeAddrs[currentNode][LOADKEY].insert(address);
    }
    else if (key == "MemCpy")
    {
        string srcStr;
        string destStr;
        string lenStr;
        stringstream streamData(value);
        getline(streamData, srcStr, ',');
        getline(streamData, destStr, ',');
        getline(streamData, lenStr, ',');
        uint64_t src = stoul(srcStr, nullptr, 0);
        uint64_t dest = stoul(destStr, nullptr, 0);
        uint64_t len = stoul(lenStr, nullptr, 0);

        for (int i=0; i <= len; i = i+4) {
            nodeAddrs[currentNode][STOREKEY].insert(dest+i);
            nodeAddrs[currentNode][LOADKEY].insert(src+i);        
        }

    }
}

map<uint64_t,int> lastWriter; 

set<tuple<int,int>> dagEdge;
void DAGColoring()
{
    int nodeNum  = kernelControlMap.size();

    for (int i = 0; i< nodeNum; i++) {

        
        for (auto addr:nodeAddrs[i][STOREKEY])
        {
            // store addrs -> lastWriter
            lastWriter[addr]= i;
        }
        // not the first node
        if (i !=0) {
            for (auto addr:nodeAddrs[i][LOADKEY])
            {
                if(lastWriter.find(addr)!=lastWriter.end())
                {
                    dagEdge.insert(tuple<int,int>(lastWriter[addr],i)); 
                }               
            }
        }
        
        
    }

}

int main(int argc, char **argv)
{

    cl::ParseCommandLineOptions(argc, argv);
    parsingKernelInfo(KernelFilename);
    ProcessTrace(InputFilename, Process, "Generating DAG", false);

    spdlog::info("DAGGenColoring start");
    DAGColoring();
    spdlog::info("DAGGenColoring end");
    for (auto i : dagEdge) {
        cout << get<0>(i) <<","<<get<1>(i)<<"\n";
    } 
    


    


    
    spdlog::info("Successfully detected kernel instance serial");
    return 0;
}

/// problem: 1.severl serial has same element but this is not same serial
//2.  -1 problem