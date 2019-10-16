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
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "json.hpp"

using namespace llvm;
using json = nlohmann::json;
using namespace std;

cl::opt<std::string> InputFilename("i", cl::desc("Specify input LLVM"), cl::value_desc("llvm filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify kernel json"), cl::value_desc("kernel filename"), cl::Required);
cl::opt<std::string> TikFilename("t", cl::desc("Specify tik json"), cl::value_desc("tik filename"), cl::Required);
//cl::opt<std::string> DagFilename("d", cl::desc("Specify DAG json"), cl::value_desc("dag filename"), cl::Required);
cl::opt<std::string> OutputLLVMFilename("o", cl::desc("Specify output bitcode"), cl::value_desc("output llvm filename"));
cl::opt<std::string> OutputJSONFilename("o2", cl::desc("Specify output JSON"), cl::value_desc("output json filename"));

static int UID = 0;
static int FUNC_ID = 0;

static int valueId = 0;

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    LLVMContext context;
    SMDiagnostic smerror;
    std::unique_ptr<Module> mptr = parseIRFile(InputFilename, smerror, context);
    Module *M = mptr.get();

    LLVMContext outContext;
    Module *OutM;
    if (OutputLLVMFilename != "") {
        OutM = new Module(OutputLLVMFilename, outContext);
    } else {
        OutM = new Module("test", outContext);
    }

    raw_fd_ostream *file = nullptr;
    if (OutputLLVMFilename != "")
    {
        std::error_code errorCode;
        file = new raw_fd_ostream(OutputLLVMFilename, errorCode);
    }

    ifstream kernelIfstream(KernelFilename);
    nlohmann::json kernelJson;
    kernelIfstream >> kernelJson;
    kernelIfstream.close();

    ifstream tikIfstream(TikFilename);
    nlohmann::json tikJson;
    tikIfstream >> tikJson;
    tikIfstream.close();

//    ifstream dagIfstream(DagFilename);
//    nlohmann::json dagJsonIn;
//    dagIfstream >> dagJsonIn;
//    dagIfstream.close();

    map<int, BasicBlock *> blockMap;
    for (Module::iterator F = M->begin(), E = M->end(); F != E; ++F)
    {
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
        {
            BasicBlock *b = cast<BasicBlock>(BB);
            blockMap[UID++] = b;
        }
    }

    IRBuilder<> builder(OutM->getContext());
    for (auto &[key, value] : kernelJson.items())
    {
        vector<int> blocks = value;
        std::sort(blocks.begin(), blocks.end());

        std::vector<Type*> arg_types = { Type::getInt8PtrTy(M->getContext()) };
        FunctionType *ftype = FunctionType::get(Type::getVoidTy(OutM->getContext()), arg_types,false);
        Function *func = Function::Create(ftype, GlobalValue::LinkageTypes::ExternalLinkage, "_KernelFunc_" + std::string(key), *OutM);
//        BasicBlock *funcEntry = BasicBlock::Create(OutM->getContext(), "_BB_entry", func);

        BasicBlock *initBlk = BasicBlock::Create(OutM->getContext(), "_BB_0", func);
        //builder.SetInsertPoint(initBlk);

        /*
        // Method built around tik to at least grab the kernel initialization block
        for (std::string allocInstr : tikJson["Kernels"]["Kernel_" + key]["Init"]) {
            //std::cout << allocInstr << std::endl;
            if (allocInstr.find("alloca i32") != std::string::npos) {
                auto *alloc = builder.CreateAlloca(Type::getInt32Ty(OutM->getContext()));
                builder.CreateStore(ConstantInt::get(Type::getInt32Ty(OutM->getContext()), 0), alloc);
//                (new AllocaInst(Type::getInt32Ty(OutM->getContext()), 0, "", initBlk));
                //initBlk->getInstList().push_back(builder.CreateAlloca(Type::getInt32Ty(OutM->getContext())));
            } else {
                std::cerr << "Unsupported allocation in kernel initialization: " << allocInstr << std::endl;
            }
        }
        */


        // In this method, we instead assume that important variable allocations are happening in the first block of our source function
        blocks.insert(blocks.begin(), 0);

        for (int block : blocks)
        {
            BasicBlock *sourceBlock = blockMap[block];
            BasicBlock *prevBB = &(func->back());
//            if (!func->empty()) {
//                prevBB = &(func->back());
//            }
            BasicBlock *bb = BasicBlock::Create(OutM->getContext(), "_BB_" + std::to_string(block), func);
//            if (!func->empty()) {
            builder.SetInsertPoint(prevBB);
            builder.CreateBr(bb);
//            }
            ValueToValueMapTy vmap;
            for (Instruction &I : *sourceBlock) {
                if (auto *br = dyn_cast<BranchInst>(&I)) {
                    continue;
                }
                Instruction *I2 = I.clone();
                vmap[&I] = I2;
//                std::cerr << "Looking at an instruction with operands: ";
//                for (unsigned int i = 0; i < I2->getNumOperands(); i++) {
//                    //std::cerr << I2->getOperand(i)->getValueName()->getKey().str() << ", ";
//                    std::cerr << I2->getOperand(i)->getName().str() << ", ";
//                    std::cerr << I2->getOperand(i)->getValueName() << ", ";
//                    sourceBlock->getValueSymbolTable();
//                    std::cerr << I2->getOperand(i)->getValueID();
//                    std::cerr << std::endl << "For reference, the instruction is: ";
//                    I2->getOperand(i)->print(errs(), true);
//                }
//                std::cerr << std::endl;
                bb->getInstList().push_back(I2);
                RemapInstruction(I2, vmap);
            }
        }
        builder.SetInsertPoint(&(func->back()));
        builder.CreateRetVoid();
        verifyFunction(*func, &outs());
    }

//    FunctionType *ftype = FunctionType::get(Type::getVoidTy(OutM->getContext()),false);
//    Function::Create(ftype, GlobalValue::LinkageTypes::ExternalLinkage, "_DASH_FFT_", *OutM);

    verifyModule(*OutM, &outs());

    if (file != nullptr) {
        OutM->print(*file, nullptr);
        file->close();
    } else {
        OutM->print(outs(), nullptr);
    }

    return 0;
}