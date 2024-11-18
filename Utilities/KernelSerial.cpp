#include "AtlasUtil/Annotate.h"
#include "AtlasUtil/Traces.h"
#include "llvm/ADT/SmallVector.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <indicators/progress_bar.hpp>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
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

using namespace llvm;
using namespace std;

// This file is used to generate the input/output relationship between kernel instances and serial of kernel instances

cl::opt<std::string> InputFilename("t", cl::desc("Specify input trace"), cl::value_desc("trace filename"), cl::Required);
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output json"), cl::value_desc("output filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify kernel json"), cl::value_desc("kernel filename"), cl::Required);
llvm::cl::opt<string> bitcodeFile("b", llvm::cl::desc("Specify bitcode name"), llvm::cl::value_desc("bitcode filename"), llvm::cl::Required);

llvm::cl::opt<bool> noBar("nb", llvm::cl::desc("No progress bar"), llvm::cl::value_desc("No progress bar"));

using wsTuple = struct wsTuple
{
    uint64_t start;
    uint64_t end;
    uint64_t byte_count;
    uint64_t ref_count;
    float reuse_distance;
    uint64_t timing;
};
using wsTupleMap = map<int64_t, wsTuple>;

map<int, wsTupleMap> loadwsTupleMap;
map<int, wsTupleMap> storewsTupleMap;
map<int, wsTupleMap> loadAfterStorewsTupleMap;

wsTupleMap loadwsTuples;
wsTupleMap storewsTuples;
wsTupleMap loadAftertorewsTuples;

set<int> loadlastHitTimeSet;
set<int> storelastHitTimeSet;
set<int> loadAfterStorelastHitTimeSet;

using livenessTuple = struct livenessTuple
{
    int64_t start;
    int64_t end;
};

// addr -> related time tuples
vector<livenessTuple> livenessTupleVec;
// addr -> tuple to be update in the vector
map<uint64_t, int> livenessUpdate;
map<int, int> maxLivenessPerKI;

class kernel
{
    vector<wsTuple> Tuples;

public:
    void addTuple(wsTuple wt)
    {
        Tuples.push_back(wt);
    }
};
class kernelInstance : kernel
{
    //private:
};
class application
{
    vector<kernelInstance> kernelInstances;

public:
    void addInstance(kernelInstance ki)
    {
        kernelInstances.push_back(ki);
    }
};

class inputInfos
{
};
class KernelSerial
{
private:
    /* data */
public:
    KernelSerial(/* args */);
    ~KernelSerial();
};

KernelSerial::KernelSerial(/* args */)
= default;

KernelSerial::~KernelSerial()
{
}

// kernel instance id
static int NodeID = 0;
// kernel id
string currentKernel = "-1";
int currentNodeID = -1;
bool noerrorInTrace;

// kernel instance id that only have internal addresses
set<int> internalSet;
// kernel instance id -> kernel id
map<int, string> kernelIdMap;
// kernel id -> basic block id
map<string, set<int>> kernelMap;

// kernel id -> control flow
map<int, set<int>> kernelControlMap;
set<int> barrierRemoval;
// for data size

map<int64_t, vector<uint64_t>> BBMemInstSize;

using AddrRange = tuple<int64_t, int64_t, int64_t, vector<uint64_t>>;
using AddrRangeMap = map<int64_t, AddrRange>;
map<int, AddrRangeMap> loadAddrRangeMapPerInstance;
map<int, AddrRangeMap> storeAddrRangeMapPerInstance;

uint64_t timing = 0;
uint64_t timingIn = 0;

uint64_t livenessTiming = 0;
set<int64_t> ValidBlock;
int vBlock;
int64_t instCounter = 0;

// for register address prediction
map<uint64_t, uint64_t> registerBuffer;
set<uint64_t> registerVariable;
uint64_t memfootPrintInKI; //memory footprint recording for register buffer cleaning

// for non-trivial merge performance testing
int NumTrivialMerge = 0;
int NumNonTrivialMerge = 0;

int peakStoreTNum = 0;
int peakLoadTNum = 0;
double ErrOverAll = 0.0;
int TupleNumChangedNonTri = 0;

int NontriOpen = true;
float NonTriErrTh = 0.5;
int NontriTh = 10;

map<int64_t, BasicBlock *> BBidToPtr;

bool overlap(wsTuple a, wsTuple b, int64_t error)
{
    return max(a.start, b.start) <= (min(a.end, b.end)) + error;
}

bool overlapDependenceChecking(wsTuple a, wsTuple b, int64_t error)
{
    return max(a.start, b.start) < (min(a.end, b.end)) + error;
}

bool included(wsTuple newer, wsTuple existed)
{
    return (newer.start >= existed.start && newer.end <= existed.end && (newer.start + newer.end - existed.end - existed.start != 0));
}


wsTuple tp_or_refactor(wsTuple a, wsTuple b)
{
    wsTuple wksTuple;

    wksTuple = (wsTuple){min(a.start, b.start), max(a.end, b.end), 0, 0, 0, 0};
    return wksTuple;
}
wsTuple tp_or(wsTuple a, wsTuple b, bool dynamic, set<int> &lastHitTimeSet)
{

    wsTuple wksTuple;
    float reuse_distance = 0;
    if (dynamic)
    {

        // last time hit tuple is not the same with current tuple
        if (a.start != timingIn && b.start != timingIn)
        {
            timing++;
            int timingRemove;
            if (a.timing == 0)
            {
                a.timing = timing;
                lastHitTimeSet.insert(timing);
                timingRemove = b.timing;
            }
            else if (b.timing == 0)
            {
                b.timing = timing;
                lastHitTimeSet.insert(timing);
                timingRemove = a.timing;
            }

            auto timingLarge = lastHitTimeSet.find(timing);
            auto timingSmall = lastHitTimeSet.find(timingRemove);

            reuse_distance = distance(timingSmall, timingLarge);

            reuse_distance = (reuse_distance + a.reuse_distance * a.ref_count + b.reuse_distance * b.ref_count) / (a.ref_count + b.ref_count);

            lastHitTimeSet.erase(timingRemove);

        } // last time hit tuple is the same with current tuple
        else
        {
            reuse_distance = (reuse_distance + a.reuse_distance * a.ref_count + b.reuse_distance * b.ref_count) / (a.ref_count + b.ref_count);
        }
        timingIn = min(a.start, b.start);
    }
    else
    {
        //Todo: this should be memory weighted
        reuse_distance = (a.reuse_distance * a.ref_count + b.reuse_distance * b.ref_count);
        reuse_distance = reuse_distance / (a.ref_count + b.ref_count);
    }
    // todo might need to be changed
    memfootPrintInKI += 8;
    wksTuple = (wsTuple){min(a.start, b.start), max(a.end, b.end), a.byte_count + b.byte_count, a.ref_count + b.ref_count, reuse_distance, timing};
    return wksTuple;
}

wsTuple tp_or_tuple(wsTuple a, wsTuple b)
{
    wsTuple wksTuple;
    wksTuple = (wsTuple){min(a.start, b.start), max(a.end, b.end), a.byte_count + b.byte_count, a.ref_count + b.ref_count, 0, 0};
    return wksTuple;
}

// online changing the map to speed up the processing
int updateRegister = 0;



void trivialMergeRefector(wsTupleMap &processMap, wsTuple t_new)
{
       // locating
    if (processMap.empty())
    {
        processMap[t_new.start] = t_new;
    }
    else if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        if (overlap(t_new, iter->second, 0))
        {
            {
                t_new = tp_or_refactor(t_new, iter->second);
                processMap[t_new.start] = t_new;
                if (t_new.start != iter->first)
                {
                    processMap.erase(iter);
                }
            }
        }
        else
        {
            processMap[t_new.start] = t_new;

        }
    }
    else
    {
        if (processMap.find(t_new.start) == processMap.end())
        {
            processMap[t_new.start] = t_new;
            auto iter = processMap.find(t_new.start);

            if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second, 0) &&
                processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or_refactor(prev(iter)->second, processMap[t_new.start]);
                processMap[prev(iter)->first] = tp_or_refactor(prev(iter)->second, next(iter)->second);
                processMap.erase(next(iter));
                processMap.erase(iter);
            }
            else if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or_refactor(prev(iter)->second, processMap[t_new.start]);
                processMap.erase(iter);
            }
            else if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[iter->first] = tp_or_refactor(iter->second, next(iter)->second);
                processMap.erase(next(iter));
            }
            else
            {
                processMap[t_new.start].timing = timing;
            }
        }
        else
        {
            auto iter = processMap.find(t_new.start);
            processMap[t_new.start] = tp_or_refactor(t_new, processMap[t_new.start]);
            if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[t_new.start] = tp_or_refactor(next(iter)->second, processMap[t_new.start]);
                processMap.erase(next(iter));
            }
        }
    }

}
void trivialMergeOptRegister(wsTupleMap &processMap, wsTuple t_new, set<int> &lastHitTimeSet)
{

    // locating
    if (processMap.empty())
    {
        timingIn = t_new.start;
        timing++;
        t_new.timing = timing;
        lastHitTimeSet.insert(timing);
        processMap[t_new.start] = t_new;
        memfootPrintInKI += t_new.ref_count;
    }
    else if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        if (overlap(t_new, iter->second, 0))
        {
            {
                t_new = tp_or(t_new, iter->second, true, lastHitTimeSet);
                processMap[t_new.start] = t_new;
                if (t_new.start != iter->first)
                {
                    processMap.erase(iter);
                }
            }
        }
        else
        {

            timingIn = t_new.start;
            timing++;
            t_new.timing = timing;
            lastHitTimeSet.insert(timing);
            processMap[t_new.start] = t_new;
            memfootPrintInKI += t_new.ref_count;
        }
    }
    else
    {
        if (processMap.find(t_new.start) == processMap.end())
        {
            processMap[t_new.start] = t_new;
            auto iter = processMap.find(t_new.start);
            // need to delete someone

            if (processMap.find(prev(iter)->first) != processMap.end() && included(t_new, prev(iter)->second))
            {
                //registerUpdate(t_new);
                // processMap.erase(t_new.start);
                // return;
            }
            if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second, 0) &&
                processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, processMap[t_new.start], true, lastHitTimeSet);
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, next(iter)->second, false, lastHitTimeSet);
                processMap.erase(next(iter));
                processMap.erase(iter);
            }
            else if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, processMap[t_new.start], true, lastHitTimeSet);
                processMap.erase(iter);
            }
            else if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[iter->first] = tp_or(iter->second, next(iter)->second, true, lastHitTimeSet);
                processMap.erase(next(iter));
            }
            else
            {
                timingIn = t_new.start;
                timing++;
                lastHitTimeSet.insert(timing);
                processMap[t_new.start].timing = timing;
                memfootPrintInKI += t_new.ref_count;
            }
        }
        else
        {
            auto iter = processMap.find(t_new.start);
            if (included(t_new, processMap[t_new.start]))
            {
                //registerUpdate(t_new);
                // return;
            }
            processMap[t_new.start] = tp_or(t_new, processMap[t_new.start], true, lastHitTimeSet);
            if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[t_new.start] = tp_or(next(iter)->second, processMap[t_new.start], false, lastHitTimeSet);
                processMap.erase(next(iter));
            }
        }
    }
}

bool CheckLoadAfterStoreref(wsTupleMap storeMap, wsTuple t_new)
{
    bool overlaps = false;
    if (storeMap.find(t_new.start) != storeMap.end())
    {
        overlaps = true;
    }
    else
    {
        storeMap[t_new.start] = t_new;
        auto iter = storeMap.find(t_new.start);
        if ((storeMap.find(next(iter)->first) != storeMap.end() && overlap(storeMap[next(iter)->first], t_new, 0)) || (prev(iter)->first != iter->first && overlap(storeMap[prev(iter)->first], t_new, 0)))
        {
            overlaps = true;
        }
    }
    return overlaps;
}

bool covered(wsTuple big, wsTuple small)
{
    
    bool result = big.start<=small.start && big.end>=small.end;

    // bool result = max(big.start, small.start) <= (min(big.end, small.end));
    return result;   

}
bool CheckLoadAfterStore(wsTupleMap &storeMap, wsTuple t_new)
{
    bool overlaps = false;
    if (storeMap.empty()) {
        return overlaps;
    }
    if (storeMap.find(t_new.start) != storeMap.end())
    {
        overlaps = covered(storeMap[t_new.start],t_new);
    }
    else
    {
        storeMap[t_new.start] = t_new;
        auto iter = storeMap.find(t_new.start);
        if (storeMap.find(next(iter)->first) != storeMap.end()) {
            overlaps = covered(storeMap[next(iter)->first],t_new);  
        }
        if (storeMap.find(prev(iter)->first) != storeMap.end()) {
            overlaps = covered(storeMap[prev(iter)->first],t_new);  
        }       
        storeMap.erase(t_new.start); 
    }
    return overlaps;
}


// node id -> bbid
map <int, int> StartBBinNode;

// node id ->bbid
map <int, int> endBBinNode;


int last_block = -1;

void ControlParse(int block)
{

    if (kernelControlMap[currentNodeID].find(block) == kernelControlMap[currentNodeID].end())
    {

        
        // end bb is not true
        if (currentNodeID>=0)
        {
            endBBinNode[currentNodeID] = last_block;              
        }
        StartBBinNode[NodeID] = block; // NodeID is the next node 
        



        


        // legacy variables for reuse distance
        timing = 0;
        timingIn = 0;
        loadlastHitTimeSet.clear();
        storelastHitTimeSet.clear();


        // for working set info
        currentNodeID = NodeID;
        NodeID++;
    }
}

// this is for nested basic blocks
stack<int> basicBlockBuff;
stack<int> instCounterBuff;
uint64_t address  = 0;
void Process(string &key, string &value)
{
    

    if (key == "BBEnter")
    {
        

        int block = stoi(value, nullptr, 0);

        
        // this is used to dealing with function calls inside bb that causes jumping into another bb before 
        // one bb ends
        basicBlockBuff.push(block);
        instCounterBuff.push(instCounter);

        instCounter = 0;
        vBlock = block;
        noerrorInTrace = BBMemInstSize.find(vBlock) != BBMemInstSize.end();
        ControlParse(block);
    }
    else if (key == "BBExit")
    {
        // store the last bb to figure out the end bb of a node
        last_block = vBlock;

        if (basicBlockBuff.size() > 1)
        {
            basicBlockBuff.pop();
            vBlock = basicBlockBuff.top();
            instCounter = instCounterBuff.top();
            instCounterBuff.pop();

            noerrorInTrace = BBMemInstSize.find(vBlock) != BBMemInstSize.end();
        }
        else
        {
            basicBlockBuff.pop();
            instCounterBuff.pop();
        }
    }
    else if (key == "LoadSize") {
        uint64_t size = stoul(value, nullptr, 0);
        if(size== 0 )
        {
            printf("LoadSize: %lu \n",size);
            exit(0);
        }

        if (address == 0) {
            printf("address: %lu \n",address);
            exit(0);
        }


        // printf("trace LoadSize: %lu \n",size);
        // printf("read LoadSize: %lu \n",BBMemInstSize[vBlock][instCounter]);

        uint64_t dataSize = size;
        wsTuple loadwksTuple;
        loadwksTuple = (wsTuple){address, address + dataSize, 0, 0, 0, 0};

        // if the load address is from the kernel's store tuple, then not counting this load
        // this currently only works for the case that the load address is completely covered by the store tuple, so the load tuple map will be larger than it should be, more dependencies will be counted
        if (!CheckLoadAfterStore(storewsTupleMap[currentNodeID], loadwksTuple))
        {
            trivialMergeRefector(loadwsTupleMap[currentNodeID], loadwksTuple);
        }

        if (loadwsTupleMap[currentNodeID].size() > peakLoadTNum)
        {
            peakLoadTNum = loadwsTupleMap[currentNodeID].size();
        }
        address = 0;
  
        
    }
    else if (key == "StoreSize") {
        uint64_t size = stoul(value, nullptr, 0);

        if(size== 0 )
        {
            printf("StoreSize: %lu \n",size);
            exit(0);
        }

        if (address == 0) {
            printf("address: %lu \n",address);
            exit(0);
        }


        uint64_t dataSize = size;
        wsTuple storewksTuple;
        storewksTuple = (wsTuple){address, address + dataSize, 0, 0, 0, 0};
        trivialMergeRefector(storewsTupleMap[currentNodeID], storewksTuple);

        if (storewsTupleMap[currentNodeID].size() > peakStoreTNum)
        {
            peakStoreTNum = storewsTupleMap[currentNodeID].size();
        }
        address = 0;
   
    
    }
    else if (key == "StoreAddress")
    {
        address = stoul(value, nullptr, 0);

    }
    else if (key == "LoadAddress")
    {
        address = stoul(value, nullptr, 0);
      
    }
    else if (key == "MemCpy")
    {
        string srcStr;
        string destStr;
        string lenStr;
        // printf("value: %s \n",value.c_str());
        stringstream streamData(value);
        getline(streamData, srcStr, ',');
        getline(streamData, destStr, ',');
        getline(streamData, lenStr, ',');
        uint64_t src = stoul(srcStr, nullptr, 0);
        uint64_t dest = stoul(destStr, nullptr, 0);
        uint64_t len = stoul(lenStr, nullptr, 0);
        // printf("src:%lu,dest:%lu,len:%lu \n",src,dest,len);

        if (noerrorInTrace)
        {

            // load tuples
            wsTuple loadwksTuple;
            loadwksTuple = (wsTuple){src, src + len, len, 1, 0, 0};

            // // if the load address is from the kernel's store tuple, then not counting this load
            if (!CheckLoadAfterStore(storewsTupleMap[currentNodeID], loadwksTuple))
            {
                trivialMergeRefector(loadwsTupleMap[currentNodeID], loadwksTuple);
            }

            wsTuple storewksTuple;
            storewksTuple = (wsTuple){dest, dest + len, len, 1, 0, 0};
            trivialMergeRefector(storewsTupleMap[currentNodeID], storewksTuple);
        }
    }
}

void print_user(Instruction *CI, map<Instruction *, tuple<int64_t, int64_t>> memInstIndexMap)
{
    for (User *U : CI->users())
    {
        if (auto *Inst = dyn_cast<Instruction>(U))
        {
            // errs() << "F is used in instruction:\n";
            errs() << *Inst << "\n";
            if (auto *stinst = dyn_cast<StoreInst>(Inst))
            {
                errs() << "bbid: " << get<0>(memInstIndexMap[Inst]) << " index: " << get<1>(memInstIndexMap[Inst]) << "\n";
            }
            print_user(Inst, memInstIndexMap);
        }
    }
}



void unionVector(AddrRange a, AddrRange b, vector<pair<int64_t, int64_t>> &result)
{
    if (result.size() > 0)
    {
        result.pop_back();
    }
    if (
        (get<0>(a) <= get<0>(b)) && (get<1>(a) <= get<0>(b)))
    {
        pair<int64_t, int64_t> r1(get<0>(a), get<1>(a));
        result.push_back(r1);
        pair<int64_t, int64_t> r2(get<0>(b), get<1>(b));
        result.push_back(r2);
    }
    else if ((get<0>(a) <= get<0>(b)) && (get<1>(a) >= get<0>(b)) && (get<1>(a) <= get<1>(b)))
    {
        pair<int64_t, int64_t> r1(get<0>(a), get<1>(b));
        result.push_back(r1);
    }
    else if ((get<0>(b) <= get<0>(a)) && (get<1>(b) >= get<0>(a)) && (get<1>(b) <= get<1>(a)))
    {
        pair<int64_t, int64_t> r1(get<0>(b), get<1>(a));
        result.push_back(r1);
    }
    else if ((get<0>(b) <= get<0>(a)) && (get<1>(b) <= get<0>(a)))
    {
        pair<int64_t, int64_t> r1(get<0>(b), get<1>(b));
        result.push_back(r1);
        pair<int64_t, int64_t> r2(get<0>(a), get<1>(a));
        result.push_back(r2);
    }
}

// calculate the intersection of load and store in one instance
void calMemUsePerIns(map<int, int64_t> &totalMemUsage, map<int, vector<pair<int64_t, int64_t>>> &addrTouchedPerInst)
{
    int64_t memU;
    // kernel instance
    map<int, AddrRangeMap> iterstore = storeAddrRangeMapPerInstance;

    // iteration in instances
    for (auto il : loadAddrRangeMapPerInstance)
    {
        // restult vector: addr range
        vector<pair<int64_t, int64_t>> resultVec;
        // load tuples in ki
        for (auto ill : il.second)
        {
            // store tuples in ki
            vector<int64_t> tobeErase;
            for (auto ils : iterstore[il.first])
            {
                if (resultVec.size() == 0)
                {
                    unionVector(ils.second, ill.second, resultVec);
                    for (auto ite : tobeErase)
                    {
                        iterstore[il.first].erase(ite);
                    }

                    break;
                }
                else if (get<0>(ils.second) <= get<0>(ill.second))
                {
                    tobeErase.push_back(ils.first);
                    unionVector(ils.second, AddrRange(resultVec.back().first, resultVec.back().second, 0, 0), resultVec);
                }
                else if (get<0>(ils.second) >= get<0>(ill.second))
                {
                    unionVector(ill.second, AddrRange(resultVec.back().first, resultVec.back().second, 0, 0), resultVec);
                    for (auto ite : tobeErase)
                    {
                        iterstore[il.first].erase(ite);
                    }
                    break;
                }
                else if (get<0>(ils.second) == get<0>(ill.second))
                {
                    if (get<1>(ils.second) >= get<1>(ill.second))
                    {
                        tobeErase.push_back(ils.first);
                        unionVector(ils.second, AddrRange(resultVec.back().first, resultVec.back().second, 0, 0), resultVec);
                    }
                    else
                    {
                        unionVector(ill.second, AddrRange(resultVec.back().first, resultVec.back().second, 0, 0), resultVec);
                        for (auto ite : tobeErase)
                        {
                            iterstore[il.first].erase(ite);
                        }
                        break;
                    }
                }
            }
            for (auto ite : tobeErase)
            {
                iterstore[il.first].erase(ite);
            }
        }
        addrTouchedPerInst[il.first] = resultVec;
        resultVec.clear();
    }

    for (auto il : addrTouchedPerInst)
    {
        memU = 0;
        for (auto ill : il.second)
        {
            memU += (ill.second - ill.first);
        }
        totalMemUsage[il.first] = memU;
    }
}

wsTuple intersectionTuple(wsTuple load, wsTuple store)
{
    wsTuple result;
    result.start = max(load.start, store.start);
    result.end = min(load.end, store.end);
    // todo updates these
    float ldr = float(result.end - result.start) / float(load.end - load.start);
    float str = float(result.end - result.start) / float(store.end - store.start);
    result.byte_count = load.byte_count * ldr + store.byte_count * str;
    result.ref_count = load.ref_count * ldr + store.ref_count * str;
    result.reuse_distance = load.reuse_distance * ldr + store.reuse_distance * str;
    result.timing = 0;
    return result;
}

wsTupleMap intersectionTupleMap(wsTupleMap load, wsTupleMap store)
{

    wsTupleMap intersecMap;
    for (auto l : load)
    {
        for (auto s : store)
        {
            if (overlap(l.second, s.second, 0))
            {
                wsTuple intersecTuple = intersectionTuple(l.second, s.second);
                intersecMap[intersecTuple.start] = intersecTuple;
            }
        }
    }
    return intersecMap;
}

void calDependency(map<int, map<int, wsTupleMap>> &dependency)
{
    for (auto il : loadwsTupleMap)
    {
        if (il.first <= 0)
        {
            continue;
        }
        else
        {
            //TODO: should be a kernel
            for (auto is : storewsTupleMap)
            {
                if (is.first <= 0)
                {
                    continue;
                }
                wsTupleMap intersecMap;
                if (is.first >= il.first)
                {
                    break;
                }
                else
                {
                    intersecMap = intersectionTupleMap(il.second, is.second);
                }
                dependency[il.first][is.first] = intersecMap;
            }
        }
    }
}

wsTupleMap Aggregate(wsTupleMap a, wsTupleMap b)
{
    wsTupleMap res;
    for (auto i : a)
    {
        res[i.first] = i.second;
    }
    for (auto i : b)
    {
        if (res.find(i.first) == res.end())
        {
            res[i.first] = i.second;
        }
        else
        {
            set<int> numbSet;
            res[i.first] = tp_or(i.second, a[i.first], false, numbSet);
        }
    }
    return res;
}
tuple<int, int, float, int> calTotalSize(wsTupleMap a, int liveness)
{
    int size = 0;
    int access = 1;
    float locality = 0;
    for (auto i : a)
    {
        size += i.second.end - i.second.start;
        locality = (locality * access + i.second.reuse_distance * i.second.ref_count);
        locality = locality / (access + i.second.ref_count);
        access += i.second.ref_count;
    }
    tuple<int, int, float, int> res(size, access, locality, liveness);
    return res;
}

map<int64_t, int> lastWriterNodeIDMap;
wsTupleMap lastWriterTupleMap;

void DepCheckRefactor(wsTuple t_new, wsTupleMap &processMap,set <int> &node)
{
    if (processMap.empty())
    {
        return;
    }
    if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        // the condition to decide (add and delete) or update
        if (overlap(t_new, iter->second, 0))
        {
            node.insert(lastWriterNodeIDMap[iter->first]);      
        }
    }
    else
    {
        if (processMap.find(t_new.start) == processMap.end())
        {
            processMap[t_new.start] = t_new;
            auto iter = processMap.find(t_new.start);
            // need to delete someone
            if (processMap.find(prev(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], prev(iter)->second, 0))
            {
                node.insert( lastWriterNodeIDMap[prev(iter)->first]);
                processMap.erase(iter);                
            }
            if (processMap.find(next(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], next(iter)->second, 0))
            {
                node.insert(lastWriterNodeIDMap[next(iter)->first]);
                processMap.erase(iter);          
            }
            processMap.erase(t_new.start);      
        }

        node.insert(lastWriterNodeIDMap[t_new.start]);    
    }
}


bool DepCheck(wsTuple t_new, wsTupleMap &processMap)
{
    if (processMap.empty())
    {
        return false;
    }
    if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        // the condition to decide (add and delete) or update
        if (overlap(t_new, iter->second, 0))
        {
            return true;
        }
    }
    else
    {
        if (processMap.find(t_new.start) == processMap.end())
        {
            processMap[t_new.start] = t_new;
            auto iter = processMap.find(t_new.start);
            // need to delete someone

            if (processMap.find(prev(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], prev(iter)->second, 0) &&
                processMap.find(next(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], next(iter)->second, 0))
            {
                return true;

            }
            if (processMap.find(prev(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], prev(iter)->second, 0))
            {
                return true;
            }
            if (processMap.find(next(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], next(iter)->second, 0))
            {
                return true;
            }
            
            return false; 
           
        }
        
        return true;
       
    }
    return false;
}

void UpdateResultTupleMap(wsTuple t_new, wsTupleMap &processMap)
{
    if (processMap.size() == 0)
    {
        processMap[t_new.start] = t_new;
        return;
    }
    else if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        // the condition to decide (add and delete) or update
        if (overlap(t_new, iter->second, 0))
        {
            wsTuple res_tuple = tp_or_tuple(t_new, iter->second);
            processMap[res_tuple.start] = res_tuple;
            if (t_new.start != iter->first)
            {
                processMap.erase(iter);
            }
        }
        else
        {
            processMap[t_new.start] = t_new;
        }
    }
    else
    {
        if (processMap.find(t_new.start) == processMap.end())
        {
            processMap[t_new.start] = t_new;
            auto iter = processMap.find(t_new.start);
            // need to delete someone

            if (processMap.find(prev(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], prev(iter)->second, 0) &&
                processMap.find(next(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or_tuple(prev(iter)->second, processMap[t_new.start]);
                processMap[prev(iter)->first] = tp_or_tuple(prev(iter)->second, next(iter)->second);
                processMap.erase(next(iter));
                processMap.erase(iter);
            }
            else if (processMap.find(prev(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], prev(iter)->second, 0))
            {
                processMap[prev(iter)->first] = tp_or_tuple(prev(iter)->second, processMap[t_new.start]);
                processMap.erase(iter);
            }
            else if (processMap.find(next(iter)->first) != processMap.end() && overlapDependenceChecking(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[iter->first] = tp_or_tuple(iter->second, next(iter)->second);
                processMap.erase(next(iter));
            }
        }
        else
        {
            auto iter = processMap.find(t_new.start);
            processMap[t_new.start] = tp_or_tuple(t_new, processMap[t_new.start]);
            if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second, 0))
            {
                processMap[t_new.start] = tp_or_tuple(next(iter)->second, processMap[t_new.start]);
                processMap.erase(next(iter));
            }
        }
    }
    return;
}

void DepCheckResTuple(wsTuple t_new, wsTupleMap processMap, wsTupleMap &resultMap)
{
    for (auto tp : processMap)
    {
        if (overlap(t_new, tp.second, 0))
        {
            wsTuple res_tuple = tp_or_tuple(t_new, tp.second);
            UpdateResultTupleMap(t_new, resultMap);
        }
    }
}

wsTuple tp_and_tuple(wsTuple a, wsTuple b)
{
    wsTuple wksTuple;
    // the byte count and ref counter are not used now, need updates
    wksTuple = (wsTuple){max(a.start, b.start), min(a.end, b.end), 0, 0, 0, 0};
    return wksTuple;
}



void SubstractResultTupleMap(wsTuple t_new, wsTupleMap &processMap)
{
    if (processMap.size() == 0)
    {
        return;
    }
    else if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        if (overlapDependenceChecking(t_new, iter->second, 0))
        {
            wsTuple and_tuple = tp_and_tuple(t_new, iter->second);
            wsTuple res_tuple;
            //3 cases, number of result tuples: 0, 1, 2
            // case 0
            if (and_tuple.start == iter->second.start && and_tuple.end == iter->second.end)
            {
                processMap.erase(iter);
            } //case 1 overlap at the beginning
            else if (and_tuple.start == iter->second.start && and_tuple.end < iter->second.end)
            {
                res_tuple = (wsTuple){and_tuple.end, iter->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
                processMap.erase(iter);
            } //case 1 overlap at the end
            else if (and_tuple.start > iter->second.start && and_tuple.end == iter->second.end)
            {
                res_tuple = (wsTuple){iter->second.start, and_tuple.start, 0, 0, 0, 0};
                processMap[iter->second.start] = res_tuple;
            } // case 2
            else if (and_tuple.start > iter->second.start && and_tuple.end < iter->second.end)
            {
                res_tuple = (wsTuple){iter->second.start, and_tuple.start, 0, 0, 0, 0};
                processMap[iter->second.start] = res_tuple;

                res_tuple = (wsTuple){and_tuple.end, iter->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
            }
        }
    }
    else
    {
        //start overlap
        if (processMap.find(t_new.start) != processMap.end())
        {
            auto start = processMap.find(t_new.start);
            // the first tuple in process map
            if (start->second.end > t_new.end)
            {
                wsTuple res_tuple = (wsTuple){t_new.end, start->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
            }

            // the following tuples in process map
            auto iter = next(start);
            while (processMap.find(iter->second.start) != processMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t_new.end <= iter->second.start)
                {
                    break;
                }
                else if (t_new.end < iter->second.end)
                {
                    wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                    processMap[res_tuple.start] = res_tuple;
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                }
                else if (t_new.end >= iter->second.end)
                {
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                }
                else
                {
                    iter = next(iter);
                }
            }
            processMap.erase(start);
        }
        else
        {
            processMap[t_new.start] = t_new;
            auto start = processMap.find(t_new.start);

            // check the tuple in process map that before substractor
            auto iter = start;
            iter = prev(iter);
            // there is one prev tuple
            if (processMap.find(iter->second.start) != processMap.end())
            {
                // overlaps
                if (iter->second.end > t_new.start)
                {
                    // 1 tuple res
                    if (iter->second.end <= t_new.end)
                    {
                        wsTuple res_tuple = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                        processMap[res_tuple.start] = res_tuple;
                    } // 2 tuple res
                    else
                    {
                        wsTuple res_tuple1 = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                        processMap[res_tuple1.start] = res_tuple1;
                        wsTuple res_tuple2 = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                        processMap[res_tuple2.start] = res_tuple2;
                    }
                }
            }

            // check the following tuples
            iter = start;
            iter = next(iter);

            while (processMap.find(iter->second.start) != processMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t_new.end <= iter->second.start)
                {
                    break;
                }
                else if (t_new.end < iter->second.end)
                {
                    wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                    processMap[res_tuple.start] = res_tuple;
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter); // need to point to next iter before delete it
                }
                else if (t_new.end >= iter->second.end)
                {
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                }
                else
                {
                    iter = next(iter);
                }
            }
            processMap.erase(start);
        }
    }
}

void SubstractionResTuple(wsTupleMap substractee, wsTupleMap substractor, wsTupleMap &resultMap)
{
    resultMap = substractee;
    for (auto tor : substractor)
    {
        SubstractResultTupleMap(tor.second, resultMap);
    }
}

bool DepCheckMaps(int checkpoint, vector<int> nodeCheckVector)
{
    bool dep = false;
    int current = checkpoint - 1;
    while (current >= 0)
    {
        for (auto i : storewsTupleMap[nodeCheckVector[current]])
        {
            dep = DepCheck(i.second, loadwsTupleMap[nodeCheckVector[checkpoint]]);
            if (dep == true)
            {
                return true;
            }
        }
        current--;
    }
    return dep;
}

void checkNodeVecDep(vector<int> nodeCheckVector)
{
    int iter = 0;
    int prevNode;

    if (nodeCheckVector.size() < 2)
    {
        return;
    }

    for (int i = 1; i < nodeCheckVector.size(); i++)
    {
        if (!DepCheckMaps(i, nodeCheckVector))
        {
            barrierRemoval.insert(nodeCheckVector[i]);
        }
    }
}

void DAGGenerationCEDR()
{
    vector<int> nodeCheckVector;
    int nonKernelCounter = 0;

    for (auto i : kernelIdMap)
    {
        if (i.second == "-1")
        {
            nonKernelCounter++;
            if (nonKernelCounter == 2)
            {
                //check the kernel nodes
                checkNodeVecDep(nodeCheckVector);

                //reset the non kernel counter
                nonKernelCounter = 0;
                //reset the kernel check set
                nodeCheckVector.clear();
            }
        }
        else
        {
            // push the node into kernel check set
            nodeCheckVector.push_back(i.first);
        }
    }
}


bool CheckNodeDep(int source, int target)
{
    int dep = false;
    for (auto i : storewsTupleMap[source])
    {
        dep = DepCheck(i.second, loadwsTupleMap[target]);
        if (dep == true)
        {
            return true;
        }
    }
    return dep;
}

// check if the store overlap of 1 and 2, overlaps with the load of node3
bool StoreSetOverlaping(int node1, int node2, int node3)
{
    bool dep = false;
    wsTupleMap Overlaps;
    for (auto i : storewsTupleMap[node1])
    {
        DepCheckResTuple(i.second, storewsTupleMap[node2], Overlaps);
    }

    for (auto i : Overlaps)
    {
        dep = DepCheck(i.second, loadwsTupleMap[node3]);
        if (dep == true)
        {
            return true;
        }
    }
    return dep;
}

set<pair<int, int>> DAGEdge;

map<int, set<int, greater<int>>> DAGPrevNodeMap;

//https://stackoverflow.com/questions/8833938/is-the-stdset-iteration-order-always-ascending-according-to-the-c-specificat
set<int, greater<int>> LiveNodeSet;

// node position for networks graph

map<int, int> NodePosition;

// find which node in the graph should the new node connect
// return node value if success else return -1
int RecursiveCheckPrevNode(int liveNode, int newNode)
{
    int checkNode = liveNode;

    if (CheckNodeDep(liveNode, newNode))
    {
        return liveNode;
    }
    else // continue the recursion
    {
        // disable the recursion, this will introduce some problem that new node depend on nodes that appeared very early
        if (DAGPrevNodeMap[checkNode].size() > 0)
        {
            for (auto prevNode : DAGPrevNodeMap[checkNode])
            {
                // problem here !!!
                int res = RecursiveCheckPrevNode(prevNode, newNode);
                if (res != -1)
                {
                    return res;
                }
            }
        }
    }
    return -1;
}

bool CheckNodeNewStoreDep(int source, wsTupleMap NewNodeLoadWS)
{
    int dep = false;
    for (auto i : storewsTupleMap[source])
    {
        dep = DepCheck(i.second, NewNodeLoadWS);
        if (dep == true)
        {
            return true;
        }
    }
    return dep;
}

void updatePrveNodeMap(int baseNode, int checkNode, map<int, set<int>> &prevNodeMap)
{
    if (checkNode == 0)
    {
        return;
    }

    for (auto prevNode : DAGPrevNodeMap[checkNode])
    {
        // problem here !!!
        prevNodeMap[baseNode].insert(prevNode);
        updatePrveNodeMap(baseNode, prevNode, prevNodeMap);
    }
}

// the subtraction inside this also need to be recursive checking like the outside one
void RecursivePrevNodeCheck(int checkNode, set<int, greater<int>> &LiveNodeCheckingSet, wsTupleMap &NewNodeLoadWS)
{

    set<int, greater<int>> tempDepPrevNodes;
    if (DAGPrevNodeMap[checkNode].size() > 0)
    {
        // reset check node
        for (auto prevNode : DAGPrevNodeMap[checkNode])
        {
            if (CheckNodeNewStoreDep(prevNode, NewNodeLoadWS))
            {
                LiveNodeCheckingSet.insert(prevNode);
                tempDepPrevNodes.insert(prevNode);
            }
        }
        // subtract here for all dep prev nodes
        for (auto depPrev : tempDepPrevNodes)
        {
            SubstractionResTuple(NewNodeLoadWS, storewsTupleMap[depPrev], NewNodeLoadWS);
        }

        // for (auto depPrev : tempDepPrevNodes)
        // {
        //     RecursivePrevNodeCheck(depPrev, LiveNodeCheckingSet, NewNodeLoadWS);
        // }
        for (auto prevNode : DAGPrevNodeMap[checkNode])
        {
            RecursivePrevNodeCheck(prevNode, LiveNodeCheckingSet, NewNodeLoadWS);
        }
    }
}

void CheckLiveNodePrevNode(int liveNode, set<int, greater<int>> &LiveNodeCheckingSet, wsTupleMap &NewNodeLoadWS)
{
    int checkNode = liveNode;

    if (CheckNodeNewStoreDep(liveNode, NewNodeLoadWS))
    {
        LiveNodeCheckingSet.insert(liveNode);
        // new node store ws substract livenode
        SubstractionResTuple(NewNodeLoadWS, storewsTupleMap[liveNode], NewNodeLoadWS);
    }
    RecursivePrevNodeCheck(liveNode, LiveNodeCheckingSet, NewNodeLoadWS);
}

// tuple overwrite
void ConstructColoringStructure(wsTuple t_new, int id, wsTupleMap &processMap, map<int64_t, int> &nodeIDMap)
{
    
    if (processMap.size() == 0)
    {
        processMap[t_new.start] = t_new;
        nodeIDMap[t_new.start] = id;
        return;
    }
    else
    {
        //dont need to check the prev tuple
        if (processMap.find(t_new.start) != processMap.end())
        { 
            auto old = processMap.find(t_new.start);
            // the first tuple in process map
            if (old->second.end > t_new.end)
            {
                wsTuple res_tuple = (wsTuple){t_new.end, old->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
                nodeIDMap[res_tuple.start] = nodeIDMap[old->second.start];
            }
            // the following tuples in process map
            auto iter = next(old);
            while (processMap.find(iter->second.start) != processMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t_new.end <= iter->second.start)
                {
                    break;
                }
                else if (t_new.end < iter->second.end)
                {
                    wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                    processMap[res_tuple.start] = res_tuple;
                    nodeIDMap[res_tuple.start]= nodeIDMap[iter->second.start];
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                    nodeIDMap.erase(deleteIter->second.start);
                }
                else if (t_new.end >= iter->second.end)
                {
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                    nodeIDMap.erase(deleteIter->second.start);
                }
                else
                {
                    iter = next(iter);
                }
            }

            processMap[t_new.start] = t_new;
            nodeIDMap[t_new.start] = id;
        }
        else
        {
            processMap[t_new.start] = t_new;
            nodeIDMap[t_new.start] = id;
            auto old = processMap.find(t_new.start);

            // check the tuple in process map that before substractor
            auto iter = old;
            iter = prev(iter);
            // there is one prev tuple
            if (processMap.find(iter->second.start) != processMap.end())
            {
                // overlaps
                if (iter->second.end > t_new.start)
                {
                    // 1 tuple res
                    if (iter->second.end <= t_new.end)
                    {
                        wsTuple res_tuple = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                        processMap[res_tuple.start] = res_tuple;
                        // node id doesnt change
                    } // 2 tuple res
                    else
                    {
                        wsTuple res_tuple1 = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                        processMap[res_tuple1.start] = res_tuple1;
                        wsTuple res_tuple2 = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                        processMap[res_tuple2.start] = res_tuple2;
                        nodeIDMap[res_tuple2.start] = nodeIDMap[res_tuple1.start];
                    }
                }
            }

            // check the following tuples
            iter = old;
            iter = next(iter);

            while (processMap.find(iter->second.start) != processMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t_new.end <= iter->second.start)
                {
                    break;
                }
                else if (t_new.end < iter->second.end)
                {
                    wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                    processMap[res_tuple.start] = res_tuple;
                    nodeIDMap[res_tuple.start]= nodeIDMap[iter->second.start];
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                    nodeIDMap.erase(deleteIter->second.start);
                }
                else if (t_new.end >= iter->second.end)
                {
                    auto deleteIter = iter;
                    iter = next(iter);
                    processMap.erase(deleteIter);
                    nodeIDMap.erase(deleteIter->second.start);
                }
                else
                {
                    iter = next(iter);
                }
            }
        }
    }
}


void ConstructColoringStructureRefactor(wsTuple t_new, int id, wsTupleMap &processMap, map<int64_t, int> &nodeIDMap) 
{
    
    if (processMap.empty())
    {
        processMap[t_new.start] = t_new;
        nodeIDMap[t_new.start] = id;
        return;
    }
    
        //dont need to check the prev tuple
    if (processMap.find(t_new.start) != processMap.end())
    { 
        auto old = processMap.find(t_new.start);
        // the first tuple in process map
        if (old->second.end > t_new.end)
        {
            wsTuple res_tuple = (wsTuple){t_new.end, old->second.end, 0, 0, 0, 0};
            processMap[res_tuple.start] = res_tuple;
            nodeIDMap[res_tuple.start] = nodeIDMap[old->second.start];
        }
        // the following tuples in process map
        auto iter = next(old);
        while (processMap.find(iter->second.start) != processMap.end())
        {
            // tuples in process map exceed the range of subtractor
            if (t_new.end <= iter->second.start)
            {
                break;
            }
            if (t_new.end < iter->second.end)
            {
                wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
                nodeIDMap[res_tuple.start]= nodeIDMap[iter->second.start];
                auto deleteIter = iter;
                iter = next(iter);
                processMap.erase(deleteIter);
                nodeIDMap.erase(deleteIter->second.start);
            }
            else if (t_new.end >= iter->second.end)
            {
                auto deleteIter = iter;
                iter = next(iter);
                processMap.erase(deleteIter);
                nodeIDMap.erase(deleteIter->second.start);
            }
            else
            {
                iter = next(iter);
            }
        }

        processMap[t_new.start] = t_new;
        nodeIDMap[t_new.start] = id;
    }
    else
    {
        processMap[t_new.start] = t_new;
        nodeIDMap[t_new.start] = id;
        auto old = processMap.find(t_new.start);

        // check the tuple in process map that before substractor
        auto iter = old;
        iter = prev(iter);
        // there is one prev tuple
        if (processMap.find(iter->second.start) != processMap.end())
        {
            // overlaps
            if (iter->second.end > t_new.start)
            {
                // 1 tuple res
                if (iter->second.end <= t_new.end)
                {
                    wsTuple res_tuple = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                    processMap[res_tuple.start] = res_tuple;
                    // node id doesnt change
                } // 2 tuple res
                else
                {
                    wsTuple res_tuple1 = (wsTuple){iter->second.start, t_new.start, 0, 0, 0, 0};
                    processMap[res_tuple1.start] = res_tuple1;
                    wsTuple res_tuple2 = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                    processMap[res_tuple2.start] = res_tuple2;
                    nodeIDMap[res_tuple2.start] = nodeIDMap[res_tuple1.start];
                }
            }
        }

        // check the following tuples
        iter = old;
        iter = next(iter);

        while (processMap.find(iter->second.start) != processMap.end())
        {
            // tuples in process map exceed the range of subtractor
            if (t_new.end <= iter->second.start)
            {
                break;
            }
            if (t_new.end < iter->second.end)
            {
                wsTuple res_tuple = (wsTuple){t_new.end, iter->second.end, 0, 0, 0, 0};
                processMap[res_tuple.start] = res_tuple;
                nodeIDMap[res_tuple.start]= nodeIDMap[iter->second.start];
                auto deleteIter = iter;
                iter = next(iter);
                processMap.erase(deleteIter);
                nodeIDMap.erase(deleteIter->second.start);
            }
            else if (t_new.end >= iter->second.end)
            {
                auto deleteIter = iter;
                iter = next(iter);
                processMap.erase(deleteIter);
                nodeIDMap.erase(deleteIter->second.start);
            }
            else
            {
                iter = next(iter);
            }
        }
    }
}



void DAGGenColoring()
{
    bool inserted = false;
    for (auto i : kernelIdMap)
    {
        for (auto tp : lastWriterTupleMap)
        {
            if(DepCheck(tp.second, loadwsTupleMap[i.first]))
            {
                int node = lastWriterNodeIDMap[tp.first];
                if(node != i.first)
                {
                    inserted = true;
                    pair<int, int> edge = {node, i.first};
                    DAGEdge.insert(edge);
                    if (NodePosition[i.first] < NodePosition[node] + 1)
                    {
                        NodePosition[i.first] = NodePosition[node] + 1;
                    }
                }
            }
        }

        for (auto st : storewsTupleMap[i.first])
        {
            ConstructColoringStructure(st.second, i.first, lastWriterTupleMap,lastWriterNodeIDMap);
        }

        if(!inserted)
        {
            NodePosition[i.first] = 0;
        }
        inserted = false;
    }
}



void LastwriterMapUpdate(wsTuple t, int id)
{

    if (lastWriterTupleMap.empty())
    {
        lastWriterTupleMap[t.start] = t;
        lastWriterNodeIDMap[t.start] = id;
    }
    else if(lastWriterTupleMap.find(t.start) != lastWriterTupleMap.end())
    {
        wsTuple t_old = lastWriterTupleMap[t.start];
        if (t_old.end > t.end)
        {
            wsTuple t_new = (wsTuple){t.end, t_old.end, 0, 0, 0, 0};
            lastWriterTupleMap[t_new.start] = t_new;
            lastWriterNodeIDMap[t_new.start] = lastWriterNodeIDMap[t_old.start];
            lastWriterTupleMap[t.start] = t;
            lastWriterNodeIDMap[t.start] = id;
        }
        else {
            auto old = lastWriterTupleMap.find(t.start);
            auto iter = next(old);
           
            while (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t.end <= iter->second.start)
                {
                    break;
                }
                if (t.end < iter->second.end)
                {
                    wsTuple res_tuple = (wsTuple){t.end, iter->second.end, 0, 0, 0, 0};
                    lastWriterTupleMap[res_tuple.start] = res_tuple;
                    lastWriterNodeIDMap[res_tuple.start]= lastWriterNodeIDMap[iter->second.start];
                    auto deleteIter = iter;
                    iter = next(iter);    
                    lastWriterNodeIDMap.erase(deleteIter->second.start);
                    lastWriterTupleMap.erase(deleteIter);
                }
                else if (t.end >= iter->second.end)
                {
                    auto deleteIter = iter;
                    iter = next(iter);
                    lastWriterNodeIDMap.erase(deleteIter->second.start);
                    lastWriterTupleMap.erase(deleteIter);   
                }
                else
                {
                    iter = next(iter);
                }
            }
            lastWriterTupleMap[t.start] = t;
            lastWriterNodeIDMap[t.start] = id;
        }
        


    }
    else
    {
        lastWriterTupleMap[t.start] = t;
        lastWriterNodeIDMap[t.start] = id;
        auto old = lastWriterTupleMap.find(t.start);

        // check the tuple in process map that before substractor
        auto iter = old;
        iter = prev(iter);
        // there is one prev tuple
        if (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
        {
            // overlaps
            if (iter->second.end > t.start)
            {
                // 1 tuple res
                if (iter->second.end <= t.end)
                {
                    wsTuple res_tuple = (wsTuple){iter->second.start, t.start, 0, 0, 0, 0};
                    lastWriterTupleMap[res_tuple.start] = res_tuple;
                    // node id doesnt change
                } // 2 tuple res
                else
                {
                    wsTuple res_tuple1 = (wsTuple){iter->second.start, t.start, 0, 0, 0, 0};
                    lastWriterTupleMap[res_tuple1.start] = res_tuple1;
                    wsTuple res_tuple2 = (wsTuple){t.end, iter->second.end, 0, 0, 0, 0};
                    lastWriterTupleMap[res_tuple2.start] = res_tuple2;
                    lastWriterNodeIDMap[res_tuple2.start] = lastWriterNodeIDMap[res_tuple1.start];
                }
            }
        }

        // check the following tuples
        iter = old;
        iter = next(iter);

        while (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
        {
            // tuples in process map exceed the range of subtractor
            if (t.end <= iter->second.start)
            {
                break;
            }
            if (t.end < iter->second.end)
            {
                wsTuple res_tuple = (wsTuple){t.end, iter->second.end, 0, 0, 0, 0};
                lastWriterTupleMap[res_tuple.start] = res_tuple;
                lastWriterNodeIDMap[res_tuple.start]= lastWriterNodeIDMap[iter->second.start];
                auto deleteIter = iter;
                iter = next(iter);
                lastWriterTupleMap.erase(deleteIter);
                lastWriterNodeIDMap.erase(deleteIter->second.start);
            }
            else if (t.end >= iter->second.end)
            {
                auto deleteIter = iter;
                iter = next(iter);
                lastWriterTupleMap.erase(deleteIter);
                lastWriterNodeIDMap.erase(deleteIter->second.start);
            }
            else
            {
                iter = next(iter);
            }
        }
    }
}

void DependenceInLastWriterMap(wsTuple t, set <int> &node)
{

    if (lastWriterTupleMap.empty())
    {
        return;
    }
    if(lastWriterTupleMap.find(t.start) != lastWriterTupleMap.end())
    {
        wsTuple t_old = lastWriterTupleMap[t.start];
        
        if (t_old.end > t.end)
        {
            node.insert(lastWriterNodeIDMap[t.start]);
        }
        else {
            node.insert(lastWriterNodeIDMap[t.start]);
            auto old = lastWriterTupleMap.find(t.start);
            auto iter = next(old);
           
            while (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
            {
                // tuples in process map exceed the range of subtractor
                if (t.end <= iter->second.start)
                {
                    break;
                }
                
                node.insert(lastWriterNodeIDMap[iter->second.start]);             
                iter = next(iter);
                
            }
        }
        
    }
    else
    {
        lastWriterTupleMap[t.start] = t;
        auto old = lastWriterTupleMap.find(t.start);

        // check the tuple in process map that before substractor
        auto iter = old;
        iter = prev(iter);
        // there is one prev tuple
        if (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
        {
            // overlaps
            if (iter->second.end > t.start)
            {
                node.insert(lastWriterNodeIDMap[iter->second.start]); 
            
            }
        }

        // check the following tuples
        iter = old;
        iter = next(iter);

        while (lastWriterTupleMap.find(iter->second.start) != lastWriterTupleMap.end())
        {
            // tuples in process map exceed the range of subtractor
            if (t.end <= iter->second.start)
            {
                break;
            }
            node.insert(lastWriterNodeIDMap[iter->second.start]); 
            iter = next(iter);
            
        }
        lastWriterTupleMap.erase(t.start);
    }


}

void DAGGenColoringRefector()
{

    int nodeNum  = kernelIdMap.size();

    for (int i = 0; i< nodeNum; i++) {


        // store tuple map should be updated later, because 1.the loads will not be considered if they are from the same node
        // 2. if the loads are still considered, they are loading from the previous node


        for (auto tp : loadwsTupleMap[i] )
        {
            set <int> node;
            // DepCheckRefactor(tp.second, lastWriterTupleMap,node);

            DependenceInLastWriterMap(tp.second, node);

            for (auto n: node) {
                if (n != i) {
                    pair<int, int> edge = {n, i};
                    DAGEdge.insert(edge);  
                } 
            }
        }



        for (auto st : storewsTupleMap[i])
        {
            //1.update the node that laster writer map points to 
            //2.update the tuple ranges
            // ConstructColoringStructureRefactor(st.second, i, lastWriterTupleMap,lastWriterNodeIDMap);
            LastwriterMapUpdate(st.second,i);

        }


        


        

        

                   
    }
}

void DAGGenNormal()
{
    bool inserted = false;
    set<int, greater<int>> tempNodeSet = LiveNodeSet;
    set<int, greater<int>> LiveNodeCheckingSet;
    
    for (auto i : kernelIdMap)
    {
        // check the dep with live node, and recursive check all the prev node of live node
        for (auto node : LiveNodeSet)
        {
            wsTupleMap NewNodeLoadWSTemp = loadwsTupleMap[i.first];
            //update the live node checking set
            CheckLiveNodePrevNode(node, LiveNodeCheckingSet, NewNodeLoadWSTemp);
        }

        wsTupleMap NewNodeLoadWSTemp = loadwsTupleMap[i.first];

        for (auto it : LiveNodeCheckingSet)
        {
            if (CheckNodeNewStoreDep(it, NewNodeLoadWSTemp))
            {
                bool nextNodeIn = false;
                // check if it is the prev node of live node then not adding edge

                if (!nextNodeIn)
                {
                    // update the edges
                    pair<int, int> edge = {it, i.first};
                    DAGEdge.insert(edge);
                    if (NodePosition[i.first] < NodePosition[it] + 1)
                    {
                        NodePosition[i.first] = NodePosition[it] + 1;
                    }

                    // update the live node
                    inserted = true;
                    DAGPrevNodeMap[i.first].insert(it);
                    tempNodeSet.insert(i.first);
                    
                    if (LiveNodeSet.find(it) != LiveNodeSet.end())
                    {
                        tempNodeSet.erase(it);
                    }
                }

                SubstractionResTuple(NewNodeLoadWSTemp, storewsTupleMap[it], NewNodeLoadWSTemp);
            }
        }

        LiveNodeSet = tempNodeSet;
        LiveNodeCheckingSet.clear();
      
        if (!inserted)
        {
            LiveNodeSet.insert(i.first);
            NodePosition[i.first] = 0;
        }
        inserted = false;
    }
}

int get_random_node(set<int> s)
{
    srand (time(NULL));
    auto r = rand()%s.size();
    auto it = begin(s);
    std::advance(it,r);
    return *it;
}


bool CheckTupleMapDep(wsTupleMap NodeStoreWS, wsTupleMap NodeLoadWS)
{
    int dep = false;
    for (auto i : NodeStoreWS)
    {
        dep = DepCheck(i.second, NodeLoadWS);
        if (dep == true)
        {
            return true;
        }
    }
    return dep;
}



vector<int> testSchedule;

void GenSchedule(set<int> &schedulableNodes,map<int,set<int>> &NextNodeMap,map<int,set<int>> &PrevNodeMap)
{
    int scheduleNode;

    while (schedulableNodes.size()>0)
    {
        scheduleNode = get_random_node(schedulableNodes);
        while(schedulableNodes.size()>1 &&scheduleNode ==kernelIdMap.size()-1)
        {
            scheduleNode = get_random_node(schedulableNodes);
        }
        
        schedulableNodes.erase(scheduleNode);
        testSchedule.push_back(scheduleNode);

        // printf("schedued node :%d \n",scheduleNode);
        // update the prev map and the next map
        for (auto it: NextNodeMap[scheduleNode])
        {
            PrevNodeMap[it].erase(scheduleNode);
            if (PrevNodeMap[it].size()==0)
            {
                // update the schedulable nodes set
                schedulableNodes.insert(it);
            } 
        }
        NextNodeMap.erase(scheduleNode);
        PrevNodeMap.erase(scheduleNode);
    }
}
void GenTestSchedule()
{
    map<int,set<int>> NextNodeMap;
    map<int,set<int>> PrevNodeMap;
    set<int> schedulableNodes;
    

    for (auto i : DAGEdge)
    {
        NextNodeMap[i.first].insert(i.second);
        PrevNodeMap[i.second].insert(i.first);
    }

    for (auto i : kernelIdMap)
    {
        if(PrevNodeMap[i.first].size()==0)
        {
            schedulableNodes.insert(i.first);
        }
    }

    GenSchedule(schedulableNodes,NextNodeMap,PrevNodeMap);

}

void CheckWAW()
{
    lastWriterTupleMap.clear();

    map<int,set<int>> NextNodeMap;


    for (auto i : DAGEdge)
    {
        NextNodeMap[i.first].insert(i.second);
    }

    // all the node in time order
    for (auto i : kernelIdMap)
    {
        // node-> memory tuple that they wrote to
        for (auto tp : lastWriterTupleMap)
        {
            if(DepCheck(tp.second, storewsTupleMap[i.first]))
            {
                int node = lastWriterNodeIDMap[tp.first];


                // if(node != i.first)
                // {      
                //     pair<int, int> edge = {node, i.first};
                //     DAGEdge.insert(edge);                  
                // }


                // calculate the overlaps
                wsTupleMap Overlaps;                
                DepCheckResTuple(tp.second,storewsTupleMap[i.first], Overlaps);
                
               
                for(auto child: NextNodeMap[i.first])
                {
                    // check dependence load the overlaps
                    if(CheckTupleMapDep(Overlaps, loadwsTupleMap[i.first]))
                    {
                        if(node != i.first)
                        {      
                            pair<int, int> edge = {node, i.first};
                            DAGEdge.insert(edge);                  
                        }
                    }
                }
                
            }
        }

        for (auto st : storewsTupleMap[i.first])
        {
            ConstructColoringStructure(st.second, i.first, lastWriterTupleMap,lastWriterNodeIDMap);
        }  
    }
}



set<pair<int, int>> SingleThreadDAGEdge;
map<int, int> SingleThreadNodePosition;


void SingThreadDAG(set<int> schedulableNonKernel,set<int> schedulableKernel,map<int,set<int>> NextNodeMap,map<int,set<int>> PrevNodeMap)
{
    int nkcounter=0;
    int prev = -1;
    // this is for updating the shedulable nodes
    map<int,set<int>> PrevNodeMapForLiveNode = PrevNodeMap;

    set<int>scheduledNodes;

    while(schedulableKernel.size()>0 ||schedulableNonKernel.size()>0)
    {
        // put non kernel into a chain
        
        
        for (auto nk:schedulableNonKernel)
        {
            if (nkcounter ==0)
            {
                prev = nk;
            }
            else
            {
                pair<int, int> edge = {prev, nk};
                SingleThreadDAGEdge.insert(edge);
                if (SingleThreadNodePosition[nk] < SingleThreadNodePosition[prev] + 1)
                {
                    SingleThreadNodePosition[nk] = SingleThreadNodePosition[prev] + 1;
                }
                prev = nk;
            }

            for (auto it: NextNodeMap[nk])
            {
                PrevNodeMapForLiveNode[it].erase(nk); 
            }
            PrevNodeMapForLiveNode.erase(nk);
            scheduledNodes.insert(nk);
            nkcounter++;
        }

        // connect kernels
        for (auto n:schedulableKernel)
        {
            if (PrevNodeMap[n].size()>0)
            {
                for (auto ne: PrevNodeMap[n])
                {
                    // kernel whose prev is non-kernel, add edge from end of current nk chain
                    if(kernelIdMap[ne]=="-1")
                    {
                        pair<int, int> edge = {prev, n};
                        SingleThreadDAGEdge.insert(edge);
                        if (SingleThreadNodePosition[n] < SingleThreadNodePosition[prev] + 1)
                        {
                            SingleThreadNodePosition[n] = SingleThreadNodePosition[prev] + 1;
                        }

                    }
                    // kernel whose prev is kernel, keep the original edge
                    else
                    {
                        pair<int, int> edge = {ne, n};
                        SingleThreadDAGEdge.insert(edge);
                        if (SingleThreadNodePosition[n] < SingleThreadNodePosition[ne] + 1)
                        {
                            SingleThreadNodePosition[n] = SingleThreadNodePosition[ne] + 1;
                        }
                    }
                }
            }

            for (auto it: NextNodeMap[n])
            {
                pair<int, int> edge = {n, it};
                SingleThreadDAGEdge.insert(edge);
                if (SingleThreadNodePosition[it] < SingleThreadNodePosition[n] + 1)
                {
                    SingleThreadNodePosition[it] = SingleThreadNodePosition[n] + 1;
                }
                PrevNodeMapForLiveNode[it].erase(n); 
            }
            PrevNodeMapForLiveNode.erase(n);
            scheduledNodes.insert(n);

        }

        // update the scheduable kernel and scheduale non-kernel
        schedulableNonKernel.clear();
        schedulableKernel.clear();

        for (auto i : kernelIdMap)
        {
            if(scheduledNodes.find(i.first)==scheduledNodes.end()&&PrevNodeMapForLiveNode[i.first].size()==0)
            {
                if(i.second=="-1")
                {
                    schedulableNonKernel.insert(i.first);
                }
                else
                {
                    schedulableKernel.insert(i.first);
                }
            }
        } 
    
    }
}



void DAGTransformSingleThread()
{

    map<int,set<int>> NextNodeMap;
    map<int,set<int>> PrevNodeMap;

    for (auto i : DAGEdge)
    {
        NextNodeMap[i.first].insert(i.second);
        PrevNodeMap[i.second].insert(i.first);
    }

    set<int> schedulableKernel;
    set<int> schedulableNonKernel;
    

    for (auto i : kernelIdMap)
    {
        if(PrevNodeMap[i.first].size()==0)
        {
            if(i.second=="-1")
            {
                schedulableNonKernel.insert(i.first);
            }
            else
            {
                schedulableKernel.insert(i.first);
            }
        }
    }

    SingThreadDAG(schedulableNonKernel,schedulableKernel,NextNodeMap,PrevNodeMap);   
}



vector <int> ScheduleForSingThread;

// start kernel stage basic block
map<int,int> startKernelIndex;

// start kernel stage basic block
map<int,int> middleKernelIndex;
// end kernel stage basic block, number of kernels counter
map<int,pair<int,int>> EndKernelIndexToCounter;

// stage index, non kernel id
map<int,set<int>> nonkernelStage;
map<int,int> nodeIndexStage;

void SingThreadSchedule(map<int, set<int>> NextNodeMap, map<int, set<int>> PrevNodeMap)
{
    int stage = 1;
    int nodeStage = 1;
    int nonkernelStageId = 1;

    set<int> schedulableKernel;
    set<int> schedulableNonKernel;

    for (auto i : kernelIdMap)
    {
        //last node
        if (i.first == kernelIdMap.size()-1)
        {
            continue;
        }
        if(PrevNodeMap[i.first].empty())
        {
            if(i.second=="-1")
            {
                schedulableNonKernel.insert(i.first);
            }
            else
            {
                schedulableKernel.insert(i.first);
            }
        }
    }
    while (!schedulableKernel.empty() || !schedulableNonKernel.empty())
    {
        // schedule the non-kernels
        while (!schedulableNonKernel.empty())
        {
            for (auto nk : schedulableNonKernel)
            {
                ScheduleForSingThread.push_back(nk);
                nodeIndexStage[nk] = nodeStage;


                nonkernelStage[nonkernelStageId].insert(nk);

                for (auto it : NextNodeMap[nk])
                {
                    PrevNodeMap[it].erase(nk);
                }
                PrevNodeMap.erase(nk);
            }
            nodeStage++;
            nonkernelStageId++;
            schedulableNonKernel.clear();

            for (auto i : kernelIdMap)
            {
                if (i.first == kernelIdMap.size()-1)
                {
                    continue;
                }
                if (PrevNodeMap.find(i.first) != PrevNodeMap.end() && PrevNodeMap[i.first].empty()&&i.second == "-1")
                {
                    schedulableNonKernel.insert(i.first);                 
                }
            }
        }
        for (auto i : kernelIdMap)
        {
            if (i.first == kernelIdMap.size()-1)
            {
                continue;
            }
            if (PrevNodeMap.find(i.first) != PrevNodeMap.end() && PrevNodeMap[i.first].empty())
            {
                if (i.second != "-1")
                {
                    schedulableKernel.insert(i.first);
                }
            }
        }

        // schedule kernels
        while (!schedulableKernel.empty())
        {
            // record how many kernels in the middle stage
            int kernelSchedCounter = 0;
            int lastSchedKernelID = -1; //?

            for (auto k : schedulableKernel)
            {
                kernelSchedCounter++;
                //
                if (kernelSchedCounter == 1)
                {
                    //stage means the schedule order of kernels? needs to be changed, why this index is the basic block?
                    startKernelIndex[StartBBinNode[k]] = stage;
                }
                else
                {
                    middleKernelIndex[StartBBinNode[k]] = stage;
                }

                ScheduleForSingThread.push_back(k);
                nodeIndexStage[k] = nodeStage;

                for (auto it : NextNodeMap[k])
                {
                    PrevNodeMap[it].erase(k);
                }
                PrevNodeMap.erase(k);
                lastSchedKernelID = k;
            }
            if (lastSchedKernelID != -1)
            {
                middleKernelIndex.erase(StartBBinNode[lastSchedKernelID]);
                EndKernelIndexToCounter[StartBBinNode[lastSchedKernelID]] = {stage, kernelSchedCounter};
                stage++;
            }

            schedulableKernel.clear();
            nodeStage++;
            for (auto i : kernelIdMap)
            {
                if (i.first == kernelIdMap.size()-1)
                {
                    continue;
                }
                if (PrevNodeMap.find(i.first) != PrevNodeMap.end() && PrevNodeMap[i.first].empty())
                {
                    if (i.second != "-1")
                    {
                        schedulableKernel.insert(i.first);
                    }
                }
            }
        }
        for (auto i : kernelIdMap)
        {
            if (i.first == kernelIdMap.size()-1)
            {
                continue;
            }
            if (PrevNodeMap.find(i.first) != PrevNodeMap.end() && PrevNodeMap[i.first].empty())
            {
                if (i.second == "-1")
                {
                    schedulableNonKernel.insert(i.first);
                }
            }
        }


    }
    ScheduleForSingThread.push_back(kernelIdMap.size()-1);
    nodeIndexStage[kernelIdMap.size()-1] = nodeStage;
}

void DAGScheduleSingleThread()
{

    map<int,set<int>> NextNodeMap;
    map<int,set<int>> PrevNodeMap;

    for (auto i : DAGEdge)
    {
        NextNodeMap[i.first].insert(i.second);
        PrevNodeMap[i.second].insert(i.first);
    }

    SingThreadSchedule(NextNodeMap,PrevNodeMap); 

}


map <int,pair<int,int>> BBMapingTransform;
void GenBBMapping()
{
    int lastNode= -1;
    for(auto i: ScheduleForSingThread)
    {
        if(lastNode != -1)
        {
            // update the node mapping
            BBMapingTransform[endBBinNode[lastNode]] = pair<int,int>{StartBBinNode[lastNode+1],StartBBinNode[i]};
        }
        lastNode = i;
    }
}

bool BBHaveStaticDependence(const BasicBlock *BB1, const BasicBlock *BB2) {
  // Iterate over the instructions in BB1
//   errs()<<"BB1:"<<BB1<<"\n";
//   errs()<<"BB2:"<<BB2<<"\n";
  for (const Instruction &Inst1 : *BB1) {
    // Check if Inst1 produces a value used by an instruction in BB2
    for (const User *U : Inst1.users()) {
      if (const Instruction *Inst2 = dyn_cast<Instruction>(U)) {
        if (Inst2->getParent() == BB2) {
          return true;
        }
      }
    }
  }
  return false;
}


set<pair<int, int>> StaticDAGEdge;

void staticNodeDependence(int prev, int later)
{
    for (auto i: kernelControlMap[prev])
    {

        for (auto j: kernelControlMap[later])
        {
            if (BBHaveStaticDependence(BBidToPtr[i], BBidToPtr[j]))
            {
                StaticDAGEdge.insert(pair<int,int>{prev,later});
                return;
            }
        }
    }
}
void StaticDAGGen()
{
    int nodenum = kernelIdMap.size();
    for(int i = 0; i<nodenum; i++)
    {
        for(int j = i+1; j<nodenum; j++)
        {
            staticNodeDependence(i,j);
        }
    }


}

int parsingKernelInfo(string KernelFilename)
{
    ifstream inputJson(KernelFilename);
    nlohmann::json j;
    inputJson >> j;
    inputJson.close();

    for (auto &[k, l] : j["Kernels"].items())
    {
        string index = k;
        nlohmann::json kernel = l["Blocks"];
        kernelMap[index] = kernel.get<set<int>>();
    }

    for (auto &[k, l] : j["Control"].items())
    {
        int index = stoul(k, nullptr, 0);
        nlohmann::json kernel = l["Blocks"];
        kernelControlMap[index] = kernel.get<set<int>>();
        kernelIdMap[index] = l["Label"];
    }

    // build memory instructions to data size map
    LLVMContext context;
    SMDiagnostic smerror;
    unique_ptr<Module> sourceBitcode;
    try
    {
        sourceBitcode = parseIRFile(bitcodeFile, smerror, context);
    }
    catch (exception &e)
    {
        return 0;
    }

    Module *M = sourceBitcode.get();
    Annotate(M);

    map<Instruction *, tuple<int64_t, int64_t>> memInstIndexMap;

    for (auto &mi : *M)
    {
        for (auto fi = mi.begin(); fi != mi.end(); fi++)
        {
            auto *bb = cast<BasicBlock>(fi);
            auto dl = bb->getModule()->getDataLayout();
            int64_t id = GetBlockID(bb);
            BBidToPtr[id] = bb;
            int64_t instIndex = 0;
            for (auto bi = fi->begin(); bi != fi->end(); bi++)
            {
                if (auto *inst = dyn_cast<LoadInst>(bi))
                {
                    auto *type = inst->getType();
                    uint64_t dataSize = dl.getTypeAllocSize(type);
                    BBMemInstSize[id].push_back(dataSize);
                    memInstIndexMap[inst] = tuple<int64_t, int64_t>{id, instIndex};
                    instIndex++;
                }
                else if (auto *inst = dyn_cast<StoreInst>(bi))
                {
                    auto *type = inst->getValueOperand()->getType();
                    uint64_t dataSize = dl.getTypeAllocSize(type);
                    BBMemInstSize[id].push_back(dataSize);
                    memInstIndexMap[inst] = tuple<int64_t, int64_t>{id, instIndex};
                    instIndex++;
                }
            }
        }
    }

    // StaticDAGGen();
    return 0;
}

int main(int argc, char **argv)
{

    

    cl::ParseCommandLineOptions(argc, argv);

    parsingKernelInfo(KernelFilename);
    application a;
    ProcessTrace(InputFilename, Process, "Generating DAG", noBar);

    
    printf("peakTupleNum %d\n", peakLoadTNum + peakStoreTNum);


    nlohmann::json jOut;
    jOut["KernelInstanceMap"] = kernelIdMap;
    // jOut["aggreatedSize"] = aggreatedSize;

    // DAGGenerationCEDR();
    
    
    spdlog::info("DAGGenColoring start");
    clock_t start_time;
    clock_t end_time;
    // DAGGenNormal();
    start_time = clock();

    DAGGenColoringRefector();
    // DAGGenColoring();
    end_time = clock();
    printf("\n time %ld \n", (end_time - start_time));

    spdlog::info("DAGGenColoring end");
    // CheckWAW();

    // GenTestSchedule();
    // GenBBMapping();
    //for transformed schedule generation
    DAGScheduleSingleThread();
    spdlog::info("DAGScheduleSingleThread end");
    //for transformed DAG generation
    // DAGTransformSingleThread();

    GenBBMapping();
    spdlog::info("GenBBMapping end");
    

    bool storeTupleInJson =true;
    if (storeTupleInJson)
    {
        for (auto sti : storewsTupleMap)
        {
            for (auto stii : sti.second)
            {
                // if (stii.second.ref_count > 1 && stii.second.byte_count > 1)
                {
                    jOut["tuplePerInstance"]["store"][to_string(sti.first)][to_string(stii.first)]["1"] = stii.second.start;
                    jOut["tuplePerInstance"]["store"][to_string(sti.first)][to_string(stii.first)]["2"] = stii.second.end;
                    jOut["tuplePerInstance"]["store"][to_string(sti.first)][to_string(stii.first)]["3"] = stii.second.byte_count;
                    jOut["tuplePerInstance"]["store"][to_string(sti.first)][to_string(stii.first)]["4"] = stii.second.ref_count;
                    jOut["tuplePerInstance"]["store"][to_string(sti.first)][to_string(stii.first)]["5"] = stii.second.reuse_distance;
                }
            }
        }
        for (auto sti : loadwsTupleMap)
        {
            for (auto stii : sti.second)
            {
                // if (stii.second.ref_count > 1 && stii.second.byte_count > 1)
                {
                    jOut["tuplePerInstance"]["load"][to_string(sti.first)][to_string(stii.first)]["1"] = stii.second.start;
                    jOut["tuplePerInstance"]["load"][to_string(sti.first)][to_string(stii.first)]["2"] = stii.second.end;
                    jOut["tuplePerInstance"]["load"][to_string(sti.first)][to_string(stii.first)]["3"] = stii.second.byte_count;
                    jOut["tuplePerInstance"]["load"][to_string(sti.first)][to_string(stii.first)]["4"] = stii.second.ref_count;
                    jOut["tuplePerInstance"]["load"][to_string(sti.first)][to_string(stii.first)]["5"] = stii.second.reuse_distance;
                }
            }
        }
    }
    
    // jOut["barrierRemoval"] = barrierRemoval;
    jOut["DAGEdge"] = DAGEdge;
    jOut["StaticDAGEdge"] = StaticDAGEdge;
    jOut["NodePosition"] = NodePosition;


    // jOut["SingleThreadDAGEdge"] = SingleThreadDAGEdge;
    // jOut["SingleThreadNodePosition"] = SingleThreadNodePosition;



    for (auto sti : StartBBinNode)
    { 
        jOut["NodeIO"][to_string(sti.first)] = {sti.second,endBBinNode[sti.first]};     
    }

    // jOut["testSchedule"] = testSchedule;



    jOut["BBMapingTransform"] = BBMapingTransform;
    jOut["ScheduleForSingThread"] = ScheduleForSingThread;

    jOut["startKernelIndex"] = startKernelIndex;
    jOut["EndKernelIndexToCounter"] = EndKernelIndexToCounter;
    jOut["middleKernelIndex"] = middleKernelIndex;
    jOut["nonkernelStage"] = nonkernelStage;
    jOut["nodeIndexStage"] = nodeIndexStage;
    
    

    std::ofstream file;
    file.open(OutputFilename);
    file << std::setw(4) << jOut << std::endl;
    file.close();

    spdlog::info("Successfully detected kernel instance serial");
    return 0;
}

/// problem: 1.severl serial has same element but this is not same serial
//2.  -1 problem