#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <tuple>
#include <vector>
#include <zlib.h>
#include <set>

using namespace std;

typedef struct wsTuple
{
    uint64_t start;
    uint64_t end; // inclusive interval
    uint64_t byte_count;
    uint64_t ref_count;
} wsTuple;

// wsTupleMap[start] = wsTuple, for fast searching and merging
typedef map<uint64_t, wsTuple> wsTupleMap;
// storeTupleMap[bbid][inst] = WStupleMap
map<uint64_t, wsTupleMap> storeTupleMap;
map<uint64_t, wsTupleMap> loadTupleMap;

uint64_t currentBB;
vector<uint64_t> BBFlow;

map<uint64_t,set<uint64_t> > BBAddrCacheLoad;
map<uint64_t,set<uint64_t>> BBAddrCacheStore;
// start parsing a new basic block

void NewBB(uint64_t bbID)
{
    BBAddrCacheLoad[currentBB].clear();
    BBAddrCacheStore[currentBB].clear();
    currentBB = bbID;
}



bool overlap(wsTuple a, wsTuple b)
{
    if (max(a.start, b.start) <= min(a.end, b.end)+1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

wsTuple tp_or(wsTuple a, wsTuple b)
{
    wsTuple wksTuple;
    wksTuple = (wsTuple){min(a.start, b.start), max(a.end, b.end), a.byte_count + b.byte_count, a.ref_count + b.ref_count};
    return wksTuple;
}

void trivialMerge(wsTupleMap &processMap, wsTuple t_new)
{
    if (processMap.size() == 0)
    {
        processMap[t_new.start] = t_new;
    }
    else if (processMap.size() == 1)
    {
        auto iter = processMap.begin();
        // the condition to decide (add and delete) or update
        if (overlap(t_new, iter->second))
        {
            t_new = tp_or(t_new, iter->second);
            processMap[t_new.start] = t_new;
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
            if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second) &&
                processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second))
            {
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, processMap[t_new.start]);
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, next(iter)->second);
                processMap.erase(next(iter));
                processMap.erase(iter);
            }
            else if (processMap.find(prev(iter)->first) != processMap.end() && overlap(processMap[t_new.start], prev(iter)->second))
            {
                processMap[prev(iter)->first] = tp_or(prev(iter)->second, processMap[t_new.start]);
                processMap.erase(iter);
            }
            else if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second))
            {
                processMap[iter->first] = tp_or(iter->second, next(iter)->second);
                processMap.erase(next(iter));
            }
        }
        else
        {
            auto iter = processMap.find(t_new.start);
            processMap[t_new.start] = tp_or(t_new, processMap[t_new.start]);
            if (processMap.find(next(iter)->first) != processMap.end() && overlap(processMap[t_new.start], next(iter)->second))
            {
                processMap[t_new.start] = tp_or(next(iter)->second, processMap[t_new.start]);
                processMap.erase(next(iter));
            }
        }
    }
}

extern "C"
{

    void MemProfInitialization()
    {
        printf("Start \n");
        currentBB = 0;
        nlohmann::json j;
          
    }


    // todo add memory cpy dump



    void StoreInstructionDump(void *address, uint64_t bbID, uint64_t datasize)
    {
        // printf("Address:%#lX, BBid: %lu, datasize:%lu \n", (uint64_t)address, bbID, datasize);
        if (bbID != currentBB)
        {
            NewBB(bbID);
        }
        uint64_t addrUInt = (uint64_t)address;
        wsTuple storewksTuple;
        

        if (BBAddrCacheStore[bbID].find(addrUInt) != BBAddrCacheStore[bbID].end())
        {
            return;
        }

        BBAddrCacheStore[bbID].insert(addrUInt);
        storewksTuple = (wsTuple){addrUInt, addrUInt + datasize-1, datasize, 1};
        trivialMerge(storeTupleMap[currentBB], storewksTuple);
        
        BBFlow.push_back(bbID);
    }

    void LoadInstructionDump(void *address, uint64_t bbID, uint64_t datasize)
    {
        // printf("Address:%#lX, BBid: %lu, datasize:%lu \n", (uint64_t)address, bbID, datasize);
        if (bbID != currentBB)
        {
            NewBB(bbID);
        }
        uint64_t addrUInt = (uint64_t)address;
        wsTuple loadwksTuple;

        if (BBAddrCacheLoad[bbID].find(addrUInt) != BBAddrCacheLoad[bbID].end())
        {
            return;
        }

        BBAddrCacheLoad[bbID].insert(addrUInt);


        loadwksTuple = (wsTuple){addrUInt, addrUInt + datasize-1, datasize, 1};
        trivialMerge(loadTupleMap[currentBB], loadwksTuple);

        BBFlow.push_back(bbID);
    }

    void MemProfDestroy()
    {
        printf("End \n");

        nlohmann::json jOut;
        std::ofstream file;
        file.open("memoryProfile.json");
        // uint64_t a = BBFlow.size();
        for (auto sti : storeTupleMap)
        {
            for (auto stii : sti.second)
            {

                    // if (stiii.second.ref_count > 1 && stiii.second.byte_count > 1)
                    {
                        jOut["MemoryProfile"]["store"][to_string(sti.first)][to_string(stii.first)]["1"] = stii.second.start;
                        jOut["MemoryProfile"]["store"][to_string(sti.first)][to_string(stii.first)]["2"] = stii.second.end;
                        jOut["MemoryProfile"]["store"][to_string(sti.first)][to_string(stii.first)]["3"] = stii.second.byte_count;
                        jOut["MemoryProfile"]["store"][to_string(sti.first)][to_string(stii.first)]["4"] = stii.second.ref_count;
                    }
                
            }
        }
        for (auto sti : loadTupleMap)
        {
            for (auto stii : sti.second)
            {

                    // if (stiii.second.ref_count > 1 && stiii.second.byte_count > 1)
                    {
                        jOut["MemoryProfile"]["load"][to_string(sti.first)][to_string(stii.first)]["1"] = stii.second.start;
                        jOut["MemoryProfile"]["load"][to_string(sti.first)][to_string(stii.first)]["2"] = stii.second.end;
                        jOut["MemoryProfile"]["load"][to_string(sti.first)][to_string(stii.first)]["3"] = stii.second.byte_count;
                        jOut["MemoryProfile"]["load"][to_string(sti.first)][to_string(stii.first)]["4"] = stii.second.ref_count;
                    }
                
            }
        }
        
        file << setw(4) << jOut << std::endl;
        file.close();
    }
}