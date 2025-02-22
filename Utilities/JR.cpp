#include "AtlasUtil/Traces.h"
#include "AtlasUtil/Annotate.h"
#include <algorithm>
#include <fstream>

#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Analysis/DependenceAnalysis.h>
#include "llvm/ADT/SmallVector.h"


using namespace llvm;
using namespace std;

cl::opt<std::string> InputFilename("i", cl::desc("Specify input trace"), cl::value_desc("trace filename"), cl::Required);
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output json"), cl::value_desc("output filename"), cl::Required);
llvm::cl::opt<bool> noBar("nb", llvm::cl::desc("No progress bar"), llvm::cl::value_desc("No progress bar"));
llvm::cl::opt<string> bitcodeFile("b", llvm::cl::desc("Specify bitcode name"), llvm::cl::value_desc("bitcode filename"), llvm::cl::Required);

map<int64_t, set<string>> labels;
string currentLabel = "-1";
int64_t currentblock;

//== for DAG node map
set <int64_t> legalBBs;
using nodeInfo = struct nodeInfo
{
    string label;
    set <int64_t> bbs;
};



map<string, int64_t> kernellabelID;


map<int64_t,nodeInfo> nodeKiidMap;
int64_t kernelIdCounter = 0;
int64_t kernelInstanceIdCounter = 0;
set <int64_t> kernelInstanceBBs;


// todo: for tasks that have not data dependencies with any other tasks,
// we need to merge them into another tasks, to maintain the control dependence
// check if the previous node is a legal non kernel
// legal non kernel means it has to have memory activities
// case 1 back to back labeled kernels
// case 2 no load and store in the interval? 
bool CheckNodeCreationIsLegal()
{
    // no blocks
    if (kernelInstanceBBs.size() ==0)
    {
        return false;
    }
    // to do add no load/store things

    for(auto i :kernelInstanceBBs)
    {
        //legal bb, there is loads and stores
        if (legalBBs.find(i)!= legalBBs.end())
        {
            return true;
        }
    }
    return false;
}
void Process(string &key, string &value)
{
    //kernel enter concludes the previous node, kernel or non-kernel
    if (key == "KernelEnter")
    {
        // kernel enter bb need to be the start bb of current node
        if (CheckNodeCreationIsLegal()){
        nodeInfo newNode = nodeInfo{currentLabel,kernelInstanceBBs};
        nodeKiidMap[kernelInstanceIdCounter] = newNode;

        kernelInstanceIdCounter++;   
        currentLabel = value;
        kernelInstanceBBs.clear();
        kernelInstanceBBs.insert(currentblock);
    }
    }
    //kernel exit concludes the previous kernel node
    else if (key == "KernelExit")
    {
    
        // adding nodes for kernels
        kernelInstanceBBs.insert(currentblock);
        nodeInfo newNode = nodeInfo{currentLabel,kernelInstanceBBs};
        nodeKiidMap[kernelInstanceIdCounter] = newNode;
        currentLabel = "-1";
        kernelInstanceBBs.clear();
        kernelInstanceIdCounter++;
    }
    else if (key == "BBEnter")
    {     
       currentblock = stol(value, nullptr, 0);
       kernelInstanceBBs.insert(currentblock);
    }
    else if (key == "NonKernelSplit")
    {
        if (CheckNodeCreationIsLegal())
        {
            kernelInstanceBBs.insert(currentblock);
            nodeInfo newNode = nodeInfo{currentLabel,kernelInstanceBBs};
            nodeKiidMap[kernelInstanceIdCounter] = newNode;
            kernelInstanceIdCounter++;   
            kernelInstanceBBs.clear();
        }
    }
}


// legal bbs mean that the bbs have memory activities
void GetLegalBBs()
{
    LLVMContext context;
    SMDiagnostic smerror;
    unique_ptr<Module> sourceBitcode;
    sourceBitcode = parseIRFile(bitcodeFile, smerror, context);
    Module *M = sourceBitcode.get();
    Annotate(M);
    for (auto &mi : *M)
    {
        for (auto fi = mi.begin(); fi != mi.end(); fi++)
        {
            auto *bb = cast<BasicBlock>(fi);
            auto dl = bb->getModule()->getDataLayout();
            int64_t id = GetBlockID(bb);
            int64_t instIndex = 0;
            for (auto bi = fi->begin(); bi != fi->end(); bi++)
            {
                if (auto *inst = dyn_cast<LoadInst>(bi))
                {
                    legalBBs.insert(id);
                    break;
                }
                else if (auto *inst = dyn_cast<StoreInst>(bi))
                {
                    legalBBs.insert(id);
                    break;
                }
                // else if (isa<CallInst>(&(*bi)) || isa<InvokeInst>(&(*bi)))
                // {
                //     legalBBs.insert(id);
                //     break;
                // }
                else if (auto *inst = dyn_cast<AllocaInst>(bi))
                {
                    legalBBs.insert(id);
                    break;
                }

                if (auto *CI = dyn_cast<CallInst>(bi)) {
                    Function *fun = CI->getCalledFunction();
                    /* The called function can be null in the event of an indirect call https://stackoverflow.com/a/11687221 */
                    if (fun) {
                        if (CI->getCalledFunction()->getName() == "malloc"||CI->getCalledFunction()->getName() == "memcpy") {
                            legalBBs.insert(id);
                            break;
                        }
                    }
                }

            }
        }
    }

}
int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    GetLegalBBs();

    ProcessTrace(InputFilename, Process, "Generating JR", noBar);

    // this is to fix the case that the last node is not counted
    if(CheckNodeCreationIsLegal())
    {
        nodeInfo newNode = nodeInfo{currentLabel,kernelInstanceBBs};
        nodeKiidMap[kernelInstanceIdCounter] = newNode;
    }

    std::ofstream file;
    nlohmann::json jOut;
 

    for (auto i : nodeKiidMap)
    {
        jOut["Control"][to_string(i.first)]["Blocks"] = i.second.bbs;
        jOut["Control"][to_string(i.first)]["Label"] = i.second.label;
    }

    // jOut["legalBBs"] = legalBBs;
  

    file.open(OutputFilename);
    file << std::setw(4) << jOut << std::endl;
    file.close();
}