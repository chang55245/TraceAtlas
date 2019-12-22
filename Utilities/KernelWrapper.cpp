#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/CommandLine.h"
#include <nlohmann/json.hpp>

using namespace llvm;
using json = nlohmann::json;
using namespace std;

cl::opt<std::string> InputFilename("i", cl::desc("Specify input bitcode"), cl::value_desc("bitcode filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify tik json"), cl::value_desc("tik filename"), cl::Required);
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output bitcode"), cl::value_desc("output filename"));

static int UID = 0;

static int valueId = 0;

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    LLVMContext context;
    SMDiagnostic smerror;
    std::unique_ptr<Module> mptr = parseIRFile(InputFilename, smerror, context);
    Module *M = mptr.get();

    map<int, BasicBlock *> blockMap;

    raw_fd_ostream *file = nullptr;
    if (OutputFilename != "")
    {
        std::error_code errorCode;
        file = new raw_fd_ostream(OutputFilename, errorCode);
    }

    for (Module::iterator F = M->begin(), E = M->end(); F != E; ++F)
    {
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
        {
            BasicBlock *b = cast<BasicBlock>(BB);
            blockMap[UID++] = b;
        }
    }

    ifstream inputJson(KernelFilename);
    nlohmann::json j;
    inputJson >> j;
    inputJson.close();

    map<string, uint64_t> outputMap;
    hash<string> hasher;
    for (auto &[key, value] : j.items())
    {
        vector<int> blocks = value;
        std::sort(blocks.begin(), blocks.end());
        vector<string> blockStrings;
        for (int block : blocks)
        {
            if (OutputFilename != "") {
                //blockMap[block]->print(errs(), true);
                //blockMap[block]->print(&file, true);
//                file << (*blockMap[block]);
                blockMap[block]->print(*file, true);
            } else {
                blockMap[block]->print(outs(), true);
            }
        }
    }

    if (file != nullptr) {
        file->close();
    }

    return 0;
}