#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <llvm/Transforms/Utils/CodeExtractor.h>
#include <llvm/Transforms/Utils/UnrollLoop.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <vector>

using namespace llvm;
using json = nlohmann::json;
using namespace std;

static int VAR_UID = 0;
static int NODE_UID = 0;

class runtime_variable {
public:
    runtime_variable() = default;

    runtime_variable(std::string name_str) {
        name = name_str;
        byteVal = std::vector<uint8_t>();
    }

    std::string name;
    uint64_t size_in_bytes = 0;
    std::vector<uint8_t> byteVal;
    bool isPtrType = false;
    uint64_t ptr_alloc_bytes = 0;
};

class parallelizable_unit;

class function_node {
public:
    function_node() = default;

    string label = "";
    int64_t kernel_id = -1;
    int64_t dagExtractor_instance_id = -1;
    Function* outlined_func;
    CallInst* call_site;
    vector<int64_t> block_indices;
    vector<BasicBlock*> block_ptrs;
    bool is_kernel = false;
    parallelizable_unit* parent_unit;
};

class parallelizable_unit {
public:
    parallelizable_unit() = default;

    function_node* non_kernel_prologue;
    function_node* kernel_node;
    function_node* non_kernel_epilogue;

    vector<parallelizable_unit*> predecessors;
    vector<parallelizable_unit*> successors;
};

StoreInst* findFirstStoreUser(Instruction* II) {
    if (auto *SI = dyn_cast<StoreInst>(II)) {
        return SI;
    }
    if (II->getNumUses() == 0) {
        return nullptr;
    }
    // First pass, check if any users directly are StoreInst. Needed for a breadth-first approach
    for (auto *itr : II->users()) {
        if (auto *storeInst = dyn_cast<StoreInst>(itr)) {
            return storeInst;
        }
    }
    // Then, check recursively only if the breadth-first check failed
    for (auto *itr : II->users()) {
        StoreInst* inst = findFirstStoreUser(dyn_cast<Instruction>(itr));
        if (inst != nullptr) {
            return inst;
        }
    }
    return nullptr;
}

uint64_t findBytesConstForMemoryAlloc(Value* val) {
    if (val == nullptr) {
        return 0;
    }
    if (auto *constant = dyn_cast<Constant>(val)) {
        if (auto *i_const = dyn_cast<ConstantInt>(constant)) {
            return i_const->getZExtValue();
        }
    }
    if (auto *storeInst = dyn_cast<StoreInst>(val)) {
        return findBytesConstForMemoryAlloc(storeInst->getValueOperand());
    }
    // Note: it is, in general, very bad to assume that memory is unchanged between a store and a load.
    // In this instance, it makes sense, however.
    if (auto *loadInst = dyn_cast<LoadInst>(val)) {
        // This makes a very strong assumption that the first store user of a load instruction's pointer operand will provide the value that would have been loaded.
        // Seeing as we're primarily applying this analysis only within the first basic block, hopefully it holds for a reasonable number of programs
        return findBytesConstForMemoryAlloc(findFirstStoreUser(dyn_cast<Instruction>(loadInst->getPointerOperand())));
    }
    if (auto *binOp = dyn_cast<BinaryOperator>(val)) {
        uint64_t op1_val = findBytesConstForMemoryAlloc(binOp->getOperand(0));
        uint64_t op2_val = findBytesConstForMemoryAlloc(binOp->getOperand(1));
        switch (binOp->getOpcode()) {
            case Instruction::Mul:
                return op1_val * op2_val;
            case Instruction::Add:
                return op1_val + op2_val;
            case Instruction::Sub:
                return op1_val - op2_val;
            case Instruction::UDiv:
                return op1_val / op2_val;
            default:
                errs() << "Encountered unknown binary operator " << binOp->getOpcodeName() << " (opcode " << binOp->getOpcode() << ")\n";
        }
    }
    return 0;
}

int64_t GetBlockID(llvm::BasicBlock *BB)
{
    int64_t result = -1;
    Instruction *first = cast<Instruction>(BB->getFirstInsertionPt());
    if (MDNode *node = first->getMetadata("BlockID"))
    {
        auto ci = cast<ConstantInt>(cast<ConstantAsMetadata>(node->getOperand(0))->getValue());
        result = ci->getSExtValue();
    }
    return result;
}

cl::opt<std::string> AnnotateFilename("a", cl::desc("Specify original input LLVM with annotated BBs"), cl::value_desc("llvm filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify kernel json"), cl::value_desc("kernel filename"), cl::Required);
cl::opt<std::string> JRFilename("j", cl::desc("Specify JR json"), cl::value_desc("jr filename"), cl::Required);
cl::opt<std::string> DagFilename("d", cl::desc("Specify DAG json"), cl::value_desc("dag filename"), cl::Required);
cl::opt<std::string> AppName("n", cl::desc("Specify name of generated application"), cl::value_desc("name"), cl::Required);
cl::opt<bool> SemanticOpt("semantic-opt", cl::desc("Enable \"semantic optimization\" of recognized kernels"), cl::Optional);
cl::opt<bool> RelaxLoops("relax-loops", cl::desc("Enable loop relaxation to help fix split loops at the cost of degraded optimization opportunities"), cl::Optional);
cl::opt<bool> UnrollNonKernels("unroll-nonkernels", cl::desc("Attempt to unroll loops that are not kernels with the hope that, if they contain kernels, they will be at the baseline part of main (and easily subbable)"), cl::Optional);
cl::opt<bool> SingleNode("single-node", cl::desc("Produce an output file with only a single node for basic testing purposes"), cl::Optional);
cl::opt<bool> LoopPartition("loop-partition", cl::desc("Partition a program into nodes based on its top level loops"), cl::Optional);
cl::opt<bool> AutoParallelize("auto-parallelize", cl::desc("Use DAG Extractor output to attempt to parallelize independent kernels"), cl::Optional);

cl::opt<std::string> OutputLLVMFilename("o", cl::desc("Specify output LLVM"), cl::value_desc("output llvm filename"));
cl::opt<std::string> OutputJSONFilename("o2", cl::desc("Specify output JSON"), cl::value_desc("output json filename"));

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    LLVMContext context;
    SMDiagnostic smerror;
    unique_ptr<Module> annotate_ptr = parseIRFile(AnnotateFilename, smerror, context);
    Module *base_module = annotate_ptr.get();
    Function* main_func = base_module->getFunction("main");

    ifstream kernelIfstream(KernelFilename);
    json kernelJson;
    kernelIfstream >> kernelJson;
    kernelIfstream.close();
    kernelJson = kernelJson["Kernels"];

    ifstream dagIfstream(DagFilename);
    nlohmann::json dagJson;
    dagIfstream >> dagJson;
    dagIfstream.close();

    ifstream jrIfstream(JRFilename);
    json jrJson;
    jrIfstream >> jrJson;
    jrIfstream.close();
    map<int64_t, string> bbToKernLabel;

    jrJson = jrJson["kernels"];

    int64_t main_start, main_end;
    map<int64_t, BasicBlock *> base_blockMap;
    map<int64_t, function_node *> kernelUID_to_function_node;
    map<int64_t, function_node *> bbToFunctionNode;
    map<int64_t, vector<int64_t>> dagPredecessorMap;

    for (Function::iterator BB = main_func->begin(), E = main_func->end(); BB != E; ++BB)
    {
        BasicBlock *b = cast<BasicBlock>(BB);
        int64_t blockID = GetBlockID(b);
        base_blockMap[blockID] = b;
        bbToFunctionNode[blockID] = nullptr;
    }

    main_start = GetBlockID(&main_func->front());
    main_end = GetBlockID(&main_func->back());

    errs() << "main starts with basic block number " << main_start << " and ends with block " << main_end << "\n";

    // Note: this loop is for processing the kernel object output from JR
    // Strip out any block from the JR json that isn't contained in main
    for (auto &item : jrJson) {
        //for (auto &blk : jrJson["blocks"]) {
        auto &blocks = item["blocks"];
        for (json::iterator blkItr = blocks.begin(); blkItr != blocks.end(); ) {
            if (*blkItr < main_start || *blkItr > main_end) {
                blocks.erase(blkItr);
            } else {
                blkItr++;
            }
        }
    }


    // Note: These two loops are used for seeding the set of kernel blocks based on the detected ontology kernels
//    map<string, vector<int64_t>> kernel_block_map;
//    vector<vector<int64_t>> kernel_blocks;
//    for (auto &[key, value] : kernelJson.items())
//    {
//        string index = key;
//        vector<int64_t> kernel = value["Blocks"];
//        kernel_block_map[index] = kernel;
//        kernel_blocks.push_back(kernel);
//    }
//
//    std::sort(std::begin(kernel_blocks), std::end(kernel_blocks), [](auto &el1, auto &el2) -> bool {
//        return el1.at(0) < el2.at(0);
//    });
//
//    // Determine which basic blocks were not classified as kernels
//    vector<int64_t> non_kernel_blocks;
//    bool is_in_kernel;
//    for (int64_t i = main_start; i <= main_end; i++) {
//        is_in_kernel = false;
//        for (const auto& kernel_elt : kernel_block_map) {
//            auto kern_indices = kernel_elt.second;
//            if (std::find(kern_indices.begin(), kern_indices.end(), i) != kern_indices.end()) {
//                is_in_kernel = true;
//                break;
//            }
//        }
//        if (!is_in_kernel){
//            non_kernel_blocks.push_back(i);
//        }
//    }

    // Note: this loop is used for seeding the set of kernel blocks based on the JR output
    vector<vector<int64_t>> kernel_blocks;
    for (const auto &item : jrJson) {
        vector<int64_t> kernel = item["blocks"];
        if (!kernel.empty()) {
            int64_t kernUID = item["globalUID"];
            function_node *node = new function_node();
            node->kernel_id = kernUID;
            node->block_indices = kernel;
            node->is_kernel = true;
            node->label = item["kernelLabel"];
            vector<BasicBlock*> block_ptrs;
            for (auto blk : kernel) {
                bbToFunctionNode[blk] = node;
                block_ptrs.push_back(base_blockMap[blk]);
            }
            node->block_ptrs = block_ptrs;
            kernel_blocks.push_back(kernel);
            kernelUID_to_function_node[kernUID] = node;
            bbToKernLabel[kernel.front()] = item["kernelLabel"];
        }
    }

    for (const auto &tuple : dagJson["KernelInstanceMap"]) {
        const auto instanceId = tuple[0];
        const auto kernID = stoul((string)tuple[1], nullptr, 0);
        if (kernelUID_to_function_node.find(kernID) != kernelUID_to_function_node.end()) {
            kernelUID_to_function_node[kernID]->dagExtractor_instance_id = instanceId;
        }
    }

    for (const auto &tuple : dagJson["ConsumerMap"]) {
        const auto instanceId = tuple[0];
        const auto predecessors = tuple[1];
        dagPredecessorMap[instanceId] = (vector<int64_t>) predecessors;
    }

    std::sort(std::begin(kernel_blocks), std::end(kernel_blocks), [](auto &el1, auto &el2) -> bool {
        return el1.at(0) < el2.at(0);
    });

    vector<int64_t> non_kernel_blocks;
    bool is_in_kernel;
    for (int64_t i = main_start; i < main_end; i++) {
        is_in_kernel = false;
        for (const auto& kernel_vec : kernel_blocks) {
            if (std::find(kernel_vec.begin(), kernel_vec.end(), i) != kernel_vec.end()) {
                is_in_kernel = true;
                break;
            }
        }
        if (!is_in_kernel){
            non_kernel_blocks.push_back(i);
        }
    }

    // Group them into contiguous ranges
    vector<vector<int64_t>> grouped_blocks;
    if (!non_kernel_blocks.empty()) {
        grouped_blocks.emplace_back();
        grouped_blocks.back().push_back(non_kernel_blocks.at(0));
        bool new_group;
        for (auto i = 1; i < non_kernel_blocks.size(); i++) {
            auto block = non_kernel_blocks.at(i);
            new_group = (block-1) != grouped_blocks.back().back() || (base_blockMap[block]->getParent() != base_blockMap[grouped_blocks.back().back()]->getParent());
            if (new_group) {
                grouped_blocks.emplace_back();
            }
            grouped_blocks.back().push_back(block);
        }
    }

    // Interleave the kernels and these blocks
    vector<pair<vector<int64_t>, bool>> interleaved_groups;
    int64_t idx1 = 0;
    int64_t idx2 = 0;
    const vector<int64_t> poison_pill{std::numeric_limits<int64_t>::max()};
    while (idx1 < kernel_blocks.size() || idx2 < grouped_blocks.size()) {
        vector<int64_t> kernel_block = (idx1 < kernel_blocks.size()) ? kernel_blocks.at(idx1) : poison_pill;
        vector<int64_t> grouped_block = (idx2 < grouped_blocks.size()) ? grouped_blocks.at(idx2) : poison_pill;
        if (kernel_block.at(0) <= grouped_block.at(0)) {
            interleaved_groups.emplace_back(kernel_block, true);
            idx1++;
        } else {
            interleaved_groups.emplace_back(grouped_block, false);
            idx2++;
        }
    }

    // Check if main's first basic block is in its own unique region or not
    // If it's not, we need to make it so
    // Its AllocaInsts mess up the rest of the program when it comes time to do code extraction currently

    if (interleaved_groups.at(0).first.size() > 1) {
        vector<int64_t> alloca_block{main_start};
        interleaved_groups.at(0).first.erase(interleaved_groups.at(0).first.begin());
        interleaved_groups.insert(interleaved_groups.begin(), pair<vector<int64_t>, bool>{alloca_block, false});
        bbToFunctionNode[main_start] = nullptr;
    }

    if (interleaved_groups.at(0).first.size() == 1 && interleaved_groups.at(0).first.at(0) == main_start) {
        interleaved_groups.erase(interleaved_groups.begin());
    }

    // All of the non-kernel blocks would not have function nodes associated with them yet
    for (auto &group : interleaved_groups) {
        if ((group.first.size() == 1 && group.first.at(0) == main_start) || group.second) {
            continue;
        }
        // We're not the first-block-of-main group or a kernel
        function_node* new_func = new function_node();
        new_func->is_kernel = false;
        new_func->kernel_id = -1;
        new_func->dagExtractor_instance_id = -1;
        new_func->label = "";
        new_func->block_indices = group.first;
        for (auto blk_idx : group.first) {
            new_func->block_ptrs.push_back(base_blockMap[blk_idx]);
            bbToFunctionNode[blk_idx] = new_func;
        }
        new_func->outlined_func = nullptr;
        new_func->parent_unit = nullptr;
    }

//    outs() << "Now, we have interleaved the kernel blocks with the non-kernel blocks, and the program structure looks like this:\n";
//    for (const auto& group : interleaved_groups) {
//        if (group.second) {
//            outs() << "Kernel: ";
//        } else {
//            outs() << "Non-kernel: ";
//        }
//        for (auto block : group.first) {
//            outs() << block << " ";
//        }
//        outs() << "\n";
//    }

    // Determine the memory requirements for all variables in this application by iterating over all the allocas
    // Only search within the first basic block, though
    map<AllocaInst*, runtime_variable*> alloca_map;
    for (BasicBlock::iterator II = base_blockMap[main_start]->begin(); II != base_blockMap[main_start]->end(); ++II) {
        if (auto *AI = dyn_cast<AllocaInst>(II)) {
            //outs() << "Found AllocaInst:" << *AI << "\n";
            alloca_map[AI] = new runtime_variable("_var_" + to_string(VAR_UID++));
            Optional<uint64_t> bits = AI->getAllocationSizeInBits(base_module->getDataLayout());
            if (bits.hasValue()) {
                if (bits.getValue() % 8 != 0) {
                    errs() << "\tThis AllocaInst requires a non-whole number of bytes. Rounding up to the nearest byte\n";
                    alloca_map[AI]->size_in_bytes = (uint64_t) std::ceil((1.0f * bits.getValue()) / 8);
                } else {
                    alloca_map[AI]->size_in_bytes = (uint64_t) bits.getValue() / 8;
                }
            } else {
                errs() << "\tThis AllocaInst that requires an unknown number of bits\n";
            }
            for (auto *user : AI->users()) {
                if (auto* SI = dyn_cast<StoreInst>(user)) {
                    if (SI->getParent() == AI->getParent()) {
                        if (auto *CONST = dyn_cast<Constant>(SI->getValueOperand())) {
                            if (auto *I_CONST = dyn_cast<ConstantInt>(CONST)) {
                                uint64_t intConst = I_CONST->getSExtValue();
                                std::vector<uint8_t> bytes;
                                for (uint64_t i = 0; i < alloca_map[AI]->size_in_bytes; i++) {
                                    bytes.push_back((uint8_t)(intConst >> (i * 8)));
                                }
                                alloca_map[AI]->byteVal = bytes;
                            }
                            else if (auto *F_CONST = dyn_cast<ConstantFP>(CONST)) {
                                uint64_t intConst = F_CONST->getValueAPF().bitcastToAPInt().getSExtValue();
                                std::vector<uint8_t> bytes;
                                for (uint64_t i = 0; i < alloca_map[AI]->size_in_bytes; i++) {
                                    bytes.push_back((uint8_t)(intConst >> (i * 8)));
                                }
                                alloca_map[AI]->byteVal = bytes;
                            }
                            else {
                                errs() << "\tThis AllocaInst is assigned a constant value, but it's of a type that I am unable to handle :(\n";
                            }
                        } else {
                            errs() << "This AllocaInst\n";
                            AI->print(errs());
                            errs() << "\nhas a value stored into it in the first basic block that I can't handle :(\n";
                            SI->print(errs());
                            errs() << "\nAdd support for propagating this value or move it outside the first basic block\n";
                        }
                    }
                }
            }
        }
    }

    // Next, iterate and find all calloc/malloc calls
    // Associate the amount they're allocating (assuming it's constant or can be found to be sorta-ish constant) with this variable
    for (BasicBlock::iterator II = base_blockMap[main_start]->begin(); II != base_blockMap[main_start]->end(); ++II) {
        if (auto *CI = dyn_cast<CallInst>(II)) {
            uint64_t bytesToAllocate = 0;
            Function* calledFunc = CI->getCalledFunction();
            bool isMallocOrCalloc = false;

            if (calledFunc->getName() == "calloc") {
                isMallocOrCalloc = true;
                uint64_t numElements = 0, sizeOfElement = 0;
                if (auto *numElConst = dyn_cast<ConstantInt>(CI->getOperand(0))) {
                    numElements = numElConst->getZExtValue();
                } else {
                    errs() << "I found a call to calloc, but the first argument wasn't an integer constant. Let's see if we can find a constant to propagate...\n";
                    if (auto *binOp = dyn_cast<BinaryOperator>(CI->getOperand(0))) {
                        numElements = findBytesConstForMemoryAlloc(binOp);
                        errs() << "I think this calloc wants " << numElements << " elements allocated\n";
                    } else {
                        errs() << "Nope, I give up! For reference, this is the operand provided to calloc: ";
                        CI->getOperand(0)->print(errs());
                        errs() << "\n";
                    }
                }

                if (auto *sizeElConst = dyn_cast<ConstantInt>(CI->getOperand(1))) {
                    sizeOfElement = sizeElConst->getZExtValue();
                } else {
                    errs() << "I found a call to calloc, but the second argument wasn't an integer constant. Let's see if we can find a constant to propagate...\n";
                    if (auto *binOp = dyn_cast<BinaryOperator>(CI->getOperand(1))) {
                        sizeOfElement = findBytesConstForMemoryAlloc(binOp);
                        errs() << "I think this calloc wants each element to have a size of " << sizeOfElement << " bytes\n";
                    } else {
                        errs() << "Nope, I give up! For reference, this is the operand provided to malloc: ";
                        CI->getOperand(0)->print(errs());
                        errs() << "\n";
                    }
                }

                if (numElements != 0 && sizeOfElement != 0) {
                    bytesToAllocate = numElements * sizeOfElement;
                }
            }
            else if (calledFunc->getName() == "malloc") {
                isMallocOrCalloc = true;

                uint64_t numBytes = 0;
                if (auto *numBytesConst = dyn_cast<ConstantInt>(CI->getOperand(0))) {
                    numBytes = numBytesConst->getZExtValue();
                } else {
                    errs() << "I found a call to malloc, but the first argument wasn't an integer constant. Let's see if we can find a constant to propagate...\n";
                    if (auto *binOp = dyn_cast<BinaryOperator>(CI->getOperand(0))) {
                        numBytes = findBytesConstForMemoryAlloc(binOp);
                        errs() << "I think this malloc wants " << numBytes << " bytes\n";
                    } else {
                        errs() << "Nope, I give up! For reference, this is the operand provided to malloc: ";
                        CI->getOperand(0)->print(errs());
                        errs() << "\n";
                    }
                }

                if (numBytes != 0) {
                    bytesToAllocate = numBytes;
                }
            }

            if (isMallocOrCalloc) {
                StoreInst *storeInst = findFirstStoreUser(dyn_cast<Instruction>(II));
                if (storeInst != nullptr) {
                    if (auto *AI = dyn_cast<AllocaInst>(storeInst->getPointerOperand())) {
                        if (AI->getAllocatedType()->isPointerTy()) {
                            alloca_map[AI]->isPtrType = true;
                            alloca_map[AI]->ptr_alloc_bytes = bytesToAllocate;
                        }
                    }
                }
            }
        }
    }

//    for (auto &[key, val] : alloca_map) {
//        outs() << "AllocaInst: ";
//        key->print(outs());
//        outs() << "\n";
//        outs() << "Was determined to be a variable with the following attributes: {\n";
//        outs() << "\tname: " << val->name << "\n";
//        outs() << "\tsize_in_bytes: " << val->size_in_bytes << "\n";
//        outs() << "\tval_bytes: [";
//        for (auto i = 0; i < val->byteVal.size(); i++) {
//            if (i == val->byteVal.size()-1) {
//                fprintf(stdout, "%hhx", val->byteVal.at(i));
//                fflush(stdout);
//            } else {
//                fprintf(stdout, "%hhx, ", val->byteVal.at(i));
//                fflush(stdout);
//            }
//        }
//        outs() << "]\n";
//        outs() << "\tis_ptr: " << (val->isPtrType ? "true" : "false") << "\n";
//        outs() << "\tptr_alloc_bytes: " << val->ptr_alloc_bytes << "\n";
//    }

    DominatorTreeAnalysis dtAnalysis;
    FunctionAnalysisManager funcAnalysisManager;
    DominatorTree dominatorTree = dtAnalysis.run(*main_func, funcAnalysisManager);

    if (RelaxLoops)
    {
        LoopInfo loopInfo(dominatorTree);
        SmallVector<BasicBlock *, 8> latchVec;
//        for (auto *loop : loopInfo.getLoopsInPreorder()) {
//            errs() << "I found a loop (loop " << loop->getName() << "):\n";
//            loop->getLoopLatches(latchVec);
//            errs() << "It has " << latchVec.size() << " loop latches:\n";
//            for (auto *latch : latchVec) {
//                latch->print(errs());
//                //errs() << "\t" << latch->getName() << "\n";
//            }
//            latchVec.clear();
//        }

        bool changed = false;
        for (auto &group : interleaved_groups)
        {
            if (group.second || group.first.empty())
            {
                // Don't modify kernels and don't try to expand empty groups
                continue;
            }
            errs() << "I am processing group with blocks: ";
            for (auto block : group.first)
            {
                errs() << block << " ";
            }
            errs() << "\n";
            changed = false;
            latchVec.clear();
            Loop *loop = nullptr;
            for (auto idx : group.first)
            {
                loop = loopInfo.getLoopFor(base_blockMap[idx]);
                if (loop != nullptr)
                {
                    break;
                }
            }
            //        Loop* loop = loopInfo.getLoopFor(base_blockMap[group.first.front()]);
            if (loop != nullptr)
            {
                //errs() << "This loop has depth: " << loop->getLoopDepth() << "\n";
                loop->getLoopLatches(latchVec);
                BasicBlock *furthestLatch = latchVec.front();
                for (auto *latch : latchVec)
                {
                    if (furthestLatch->getName() < latch->getName())
                    {
                        furthestLatch = latch;
                    }
                }
                auto currentEndBlock = group.first.back();
                auto latchBoxIdx = group.first.back();
                try
                {
                    latchBoxIdx = GetBlockID(furthestLatch);
                    //latchBoxIdx = std::stoi(furthestLatch->getName().str().substr(7));
                }
                catch (exception &err)
                {
                    //errs() << "oh no this block wasn't in the form BB_UID_### :((((\n";
                    errs() << "oh no I couldn't get the block id for some reason :((((\n";
                }
                for (auto idx = currentEndBlock + 1; idx <= latchBoxIdx; idx++)
                {
                    changed = true;
                    group.first.push_back(idx);
                    for (auto &other_group : interleaved_groups)
                    {
                        if (other_group == group)
                        {
                            continue;
                        }
                        for (auto other_idx = other_group.first.begin(), other_end = other_group.first.end(); other_idx != other_end;)
                        {
                            if (*other_idx == idx)
                            {
                                other_group.first.erase(other_idx);
                                break;
                            }
                            else
                            {
                                other_idx++;
                            }
                        }
                    }
                }
            }
            if (changed)
            {
                errs() << "I expanded this group to be blocks: ";
                for (auto block : group.first)
                {
                    errs() << block << " ";
                }
                errs() << "\n";
            }
        }
    }
    else if (UnrollNonKernels) {
        LoopInfo loopInfo(dominatorTree);
        SmallVector<BasicBlock *, 8> latchVec;


        for (auto &group : interleaved_groups)
        {
            if (group.second || group.first.empty())
            {
                // Don't process kernels or empty groups
                continue;
            }
            latchVec.clear();
            Loop *loop = nullptr;

            for (auto idx : group.first)
            {
                loop = loopInfo.getLoopFor(base_blockMap[idx]);
                if (loop == nullptr)
                {
                    continue;
                }
                UnrollLoopOptions ULO{.Force = true};
                LoopUnrollResult res = UnrollLoop(loop, ULO, &loopInfo, nullptr, &dominatorTree, nullptr, nullptr, false, nullptr);
                switch (res) {
                    case LoopUnrollResult::Unmodified:
                        errs() << "This loop was unmodified!\n";
                        break;
                    case LoopUnrollResult::PartiallyUnrolled:
                        errs() << "This loop was partially modified!\n";
                        break;
                    case LoopUnrollResult::FullyUnrolled:
                        errs() << "This loop was fully unrolled!\n";
                        break;
                }
            }
        }
    }
    else if (LoopPartition) {
        LoopInfo loopInfo(dominatorTree);
        SmallVector<BasicBlock *, 8> exitVec;

        vector<pair<vector<int64_t>, bool>> new_interleaved_groups;
        new_interleaved_groups.push_back({{}, false});
        bool inLoop = false;

        for (int64_t id = main_start+1; id <= main_end;) {
            Loop* loop = loopInfo.getLoopFor(base_blockMap[id]);
            if (loop == nullptr) {
                new_interleaved_groups.back().first.push_back(id);
                id++;
                continue;
            }
            if (loop->getLoopDepth() == 1) {
                loop->getExitBlocks(exitVec);
                int64_t furthestExit = GetBlockID(exitVec.front());
                for (auto *block : exitVec) {
                    if (GetBlockID(block) > furthestExit) {
                        furthestExit = GetBlockID(block);
                    }
                }
                new_interleaved_groups.push_back({{}, true});
                for (int64_t id2 = id; id2 < furthestExit; id2++) {
                    new_interleaved_groups.back().first.push_back(id2);
                }
                new_interleaved_groups.push_back({{}, false});
                exitVec.clear();
                id = furthestExit;
            } else {
                errs() << "I found a loop with loop depth != 1, skipping processing...\n";
            }
        }
        interleaved_groups = new_interleaved_groups;
    }

    if (SingleNode) {
        interleaved_groups.clear();
        std::vector<int64_t> all_blocks;
        // Skip the first node of main
        for (int64_t id = main_start+1; id <= main_end; id++) {
            all_blocks.push_back(id);
        }
        interleaved_groups.emplace_back(all_blocks, false);
    }

    // Remove all CallInsts to KernelEnter/KernelExit
    IRBuilder<> builder(&*main_func->begin());
    for (inst_iterator I = inst_begin(main_func), E = inst_end(main_func); I != E;) {
        Instruction *inst = &*I; I++;
        if (auto *CI = dyn_cast<CallInst>(inst)) {
            if (CI->getCalledFunction()->getName() == "KernelEnter" || CI->getCalledFunction()->getName() == "KernelExit") {
                auto *thisNode = CI->getParent();
                auto *prevNode = thisNode->getSinglePredecessor();
                auto *prevTerm = prevNode->getTerminator();
                prevTerm->eraseFromParent();
                builder.SetInsertPoint(prevNode);
                builder.CreateBr(thisNode);
            }
        }
    }
    // Replace KernelEnter and KernelExit calls in every function in the module
    for (Module::iterator MM = base_module->begin(); MM != base_module->end(); MM++) {
        Function *current_func = &*MM;
        for (inst_iterator I = inst_begin(current_func), E = inst_end(current_func); I != E;) {
            Instruction *inst = &*I; I++;
            if (auto *CI = dyn_cast<CallInst>(inst)) {
                if (CI->getCalledFunction()->getName() == "KernelEnter" || CI->getCalledFunction()->getName() == "KernelExit") {
                    errs() << "Replacing a call to " << CI->getCalledFunction()->getName() << " with a nop addition\n";
                    Value *val1 = ConstantInt::get(Type::getInt32Ty(current_func->getContext()), 0);
                    Value *val2 = ConstantInt::get(Type::getInt32Ty(current_func->getContext()), 0);
                    BinaryOperator *op = BinaryOperator::Create(Instruction::Add, val1, val2);
                    ReplaceInstWithInst(inst, op);
                }
            }
        }
    }

    // Use the CodeExtractor to extract each snippet into a "node" function
    bool successful;
    map<string, pair<Function*, string>> outlined_functions;
    deque<Function*> outlined_functions_deque;
    //vector<Function*> outlined_functions;
    vector<BasicBlock*> blocks;
    for (std::size_t i = 0; i < interleaved_groups.size(); i++) {
        successful = false;
        auto& group = interleaved_groups.at(i);
        if (group.first.empty() || base_blockMap[group.first.front()] == nullptr || base_blockMap[group.first.front()]->getParent()->getName() != main_func->getName()) {
            continue;
        }
        blocks.clear();
        for (auto idx : group.first) {
            blocks.push_back(base_blockMap[idx]);
        }
        CodeExtractor CE(ArrayRef<BasicBlock *>(blocks), &dominatorTree, false, nullptr, nullptr, nullptr, false, false, "Node_" + to_string(NODE_UID));
        //CodeExtractor CE(ArrayRef<BasicBlock *>(blocks), nullptr, false, nullptr, nullptr, nullptr, false, false, "Node_" + to_string(NODE_UID));
        SetVector<Value *> Inputs, Outputs, Sinks;
        CE.findInputsOutputs(Inputs, Outputs, Sinks);

        outs() << "Attempting to outline blocks ";
        for (auto idx : group.first) {
            outs() << idx << " ";
        }
        //outs() << "\n";
        outs() << (group.second ? "(kernel)\n" : "(non-kernel)\n");
        Function *OrigF = blocks.at(0)->getParent();
        if (Function *OutF = CE.extractCodeRegion()) {
            OutF->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
            User *U = *OutF->user_begin();
            CallInst *CI = cast<CallInst>(U);
            CallSite CS(CI);
            CI->setIsNoInline();

            successful = true;
            NODE_UID++;

            // If this function has an associated kernel label, propogate it
            bbToFunctionNode[group.first.front()]->outlined_func = OutF;
            if (bbToKernLabel.find(group.first.front()) != bbToKernLabel.end()) {
                outlined_functions[OutF->getName()] = {OutF, bbToKernLabel.at(group.first.front())};
            } else {
                outlined_functions[OutF->getName()] = {OutF, ""};
            }
            outlined_functions_deque.push_back(OutF);
        }
        if (successful) {
            outs() << "Successfully outlined region as " << outlined_functions_deque.back()->getName() << "\n";
        } else {
            outs() << "Failed to extract region\n";
        }
    }

    unsigned int call_idx = 0;
    bool knownKernelReplaced = false;

    nlohmann::json outputJson;
    outputJson["AppName"] = AppName;
    outputJson["SharedObject"] = AppName + ".so";

    nlohmann::json outputVariablesJson = json::object();
    for (auto &[key, val] : alloca_map) {
        nlohmann::json varJson = json::object();
        varJson["bytes"] = val->size_in_bytes;
        varJson["val"] = json::array();
        for (uint8_t i : val->byteVal) {
            varJson["val"].push_back((uint32_t)i);
        }
        varJson["is_ptr"] = val->isPtrType;
        varJson["ptr_alloc_bytes"] = val->ptr_alloc_bytes;
        outputVariablesJson[val->name] = varJson;
    }
    // Add a placeholder for main's return value since it isn't found as an AllocaInst for whatever reason
    outputVariablesJson["_var_ret"] = json::object();
    outputVariablesJson["_var_ret"]["bytes"] = 4;
    outputVariablesJson["_var_ret"]["val"] = json::array();
    outputVariablesJson["_var_ret"]["is_ptr"] = false;
    outputVariablesJson["_var_ret"]["ptr_alloc_bytes"] = 0;

    nlohmann::json outputDagJson = json::object();

    call_idx = 0;

    CallInst *lastCallInst = nullptr;
    for (auto &BB : *main_func) {
        for (BasicBlock::iterator II = BB.begin(); II != BB.end(); ++II) {
            if (auto *CI = dyn_cast<CallInst>(II)) {
                if (outlined_functions.find(CI->getCalledFunction()->getName()) != outlined_functions.end()) {
                    lastCallInst = CI;
                }
            }
        }
    }

    if (AutoParallelize) {
        // TODO: One approach for auto parallelization would be to do a two pass process
        // TODO: A single pass is difficult as we likely want to position the non-kernel nodes based on whether or not their following
        // TODO: kernel node is independent from the preceding kernel node (as the non-kernel code is probably setup for the next kernel)
        // TODO: In the first pass, we assume that each non-kernel section is a successor to the previous kernel
        // TODO: But, if when processing the next kernel node, we notice it's independent from the previous kernel, then we unlink
        // TODO: the preceding non-kernel node and make its predecessor the furthest possible node that is a prerequisite for this kernel
        // TODO: (with the ultimate limit being basically the top of main)
        // TODO: Then, in the second pass, with this analysis complete, we generate the output JSON by walking along this graph,
        // TODO: generating the JSON for each node, and linking up its predecessors and successors as specified by the graph
        // TODO: Aside: would it benefit this processing to have some kind of struct that combines pairs of non-kernel and kernel nodes?
        // TODO: A kernel and its prologue, perhaps?

        vector<function_node*> callSequences;
        vector<parallelizable_unit*> generation_units;

        errs() << "Looking at all of the calls in main and assigning the outlined functions to the function nodes\n";
        for (auto &BB : *main_func) {
            for (BasicBlock::iterator II = BB.begin(); II != BB.end(); ++II) {
                if (auto* CI = dyn_cast<CallInst>(II)) {
                    if (outlined_functions.find(CI->getCalledFunction()->getName()) == outlined_functions.end()) {
                        //errs() << "Just so you know, there was a function call to " << CI->getCalledFunction()->getName() << " that I don't have in my map of extracted functions\n";
                        continue;
                    }
                    knownKernelReplaced = false;
                    auto *called_func = CI->getCalledFunction();
                    function_node *func_node = nullptr;
                    for (auto &[key, val] : bbToFunctionNode) {
                        if (val == nullptr) {
                            continue;
                        }
                        if (val->outlined_func == called_func) {
                            func_node = val;
                            val->call_site = CI;
                            break;
                        }
                    }
                    if (func_node == nullptr) {
                        errs() << "[ERROR] We encountered an outlined function that does not have an associated function node pointer!\n";
                    } else {
                        callSequences.push_back(func_node);
                    }
                }
            }
        }

        // Iterate and form the parallelizable units
        errs() << "Iterating over call sequences to form initial parallelization units\n";
        bool isKernel = false;
        parallelizable_unit *current_unit = nullptr;
        for (auto *call : callSequences) {
            if (call->is_kernel != isKernel) {
                errs() << "This program doesn't follow a strict sequence of alternating non-kernel, kernel! Giving up\n";
                break;
            } else {
                if (!isKernel) {
                    errs() << "Generating parallelizable unit\n";
                    current_unit = new parallelizable_unit();
                    current_unit->non_kernel_prologue = call;
                    call->parent_unit = current_unit;
                    generation_units.push_back(current_unit);
                } else {
                    generation_units.back()->kernel_node = call;
                    call->parent_unit = generation_units.back();
                }
                isKernel = !isKernel;
            }
        }

        // Iterate again and link the predecessors and successors together
        errs() << "Iterating over the parallelization units and linking them together\n";
        for (auto *unit : generation_units) {
            auto *kernel_node = unit->kernel_node;
            if (kernel_node == nullptr) {
                continue;
            }
            for (auto pred : dagPredecessorMap[kernel_node->dagExtractor_instance_id]) {
                auto *pred_unit = kernelUID_to_function_node[pred]->parent_unit;
                unit->predecessors.push_back(pred_unit);
                pred_unit->successors.push_back(unit);
            }
        }

        // Special case: if the last region has no kernel, then link it up manually since we may end without a kernel for it
        if (generation_units.size() >= 2) {
            auto *last_unit = generation_units.rbegin()[0];
            auto *second_last_unit = generation_units.rbegin()[1];
            if (last_unit->kernel_node == nullptr) {
                second_last_unit->successors.push_back(last_unit);
                last_unit->predecessors.push_back(second_last_unit);
            }
        }

        errs() << "Printing out the formed generation units\n";
        for (auto unit_num = 0; unit_num < generation_units.size(); unit_num++) {
            auto *unit = generation_units.at(unit_num);
            errs() << "Unit " << unit_num << " (ptr: " << unit << ")\n";
            if (unit->non_kernel_prologue != nullptr) {
                errs() << "\tNon-kernel prologue:\n\t\t[";
                for (auto blk_idx : unit->non_kernel_prologue->block_indices) {
                    errs() << blk_idx << ", ";
                }
                errs() << "]\n";
            }
            if (unit->kernel_node != nullptr) {
                errs() << "\tKernel (" << unit->kernel_node->kernel_id << "):\n\t\t[";
                for (auto blk_idx : unit->kernel_node->block_indices) {
                    errs() << blk_idx << ", ";
                }
                errs() << "]\n";
            }
            errs() << "\tPredecessors: [";
            for (auto *pred_unit : unit->predecessors) {
                errs() << pred_unit << ", ";
            }
            errs() << "]\n";
            errs() << "\tSuccessors: [";
            for (auto *succ_unit : unit->successors) {
                errs() << succ_unit << ", ";
            }
            errs() << "]\n";
        }

        errs() << "Generating the output DAG";
        // outputDagJson
        for (auto *unit : generation_units) {
            // Generate the non-kernel
            if (unit->non_kernel_prologue != nullptr) {
                nlohmann::json nodeNonKernelJson = json::object();
                nodeNonKernelJson["arguments"] = json::array();
                nodeNonKernelJson["predecessors"] = json::array();
                nodeNonKernelJson["successors"] = json::array();
                nodeNonKernelJson["platforms"] = json::array();

                auto *func = unit->non_kernel_prologue->outlined_func;
                auto *callInst = unit->non_kernel_prologue->call_site;

                for (size_t opNum = 0; opNum < callInst->getNumOperands()-1; opNum++) {
                    auto *arg = callInst->getOperand(opNum);
                    if (auto* AI = dyn_cast<AllocaInst>(arg)) {
                        if (alloca_map.find(AI) == alloca_map.end()) {
                            errs() << "Encountered an alloca instruction that I don't have in my map. Assuming it's the return value\n";
                            nodeNonKernelJson["arguments"].push_back("_var_ret");
                        } else {
                            nodeNonKernelJson["arguments"].push_back(alloca_map[AI]->name);
                        }
                    } else {
                        errs() << "Encountered a function with a non-alloca operand. Function: " << func->getName() << ", Operand: ";
                        arg->print(errs());
                        errs() << "\n";
                    }
                }

                for (auto *pred_unit : unit->predecessors) {
                    nlohmann::json predJson = json::object();
                    predJson["name"] = pred_unit->kernel_node->outlined_func->getName();
                    predJson["edgecost"] = 10;
                    nodeNonKernelJson["predecessors"].push_back(predJson);
                }
                if (unit->kernel_node != nullptr) {
                    nlohmann::json succJson = json::object();
                    succJson["name"] = unit->kernel_node->outlined_func->getName();
                    succJson["edgecost"] = 10;
                    nodeNonKernelJson["successors"].push_back(succJson);
                }

                nlohmann::json plat = json::object();
                plat["name"] = "cpu";
                plat["nodecost"] = 10;
                plat["runfunc"] = func->getName();
                nodeNonKernelJson["platforms"].push_back(plat);

                outputDagJson[func->getName()] = nodeNonKernelJson;
            }
            // Generate the kernel
            if (unit->kernel_node != nullptr) {
                nlohmann::json nodeKernelJson = json::object();
                nodeKernelJson["arguments"] = json::array();
                nodeKernelJson["predecessors"] = json::array();
                nodeKernelJson["successors"] = json::array();
                nodeKernelJson["platforms"] = json::array();

                auto *func = unit->kernel_node->outlined_func;
                auto *callInst = unit->kernel_node->call_site;

                for (size_t opNum = 0; opNum < callInst->getNumOperands()-1; opNum++) {
                    auto *arg = callInst->getOperand(opNum);
                    if (auto* AI = dyn_cast<AllocaInst>(arg)) {
                        if (alloca_map.find(AI) == alloca_map.end()) {
                            errs() << "Encountered an alloca instruction that I don't have in my map. Assuming it's the return value\n";
                            nodeKernelJson["arguments"].push_back("_var_ret");
                        } else {
                            nodeKernelJson["arguments"].push_back(alloca_map[AI]->name);
                        }
                    } else {
                        errs() << "Encountered a function with a non-alloca operand. Function: " << func->getName() << ", Operand: ";
                        arg->print(errs());
                        errs() << "\n";
                    }
                }

                nlohmann::json predJson = json::object();
                predJson["name"] = unit->non_kernel_prologue->outlined_func->getName();
                predJson["edgecost"] = 10;
                nodeKernelJson["predecessors"].push_back(predJson);
                for (auto *succ_unit : unit->successors) {
                    nlohmann::json succJson = json::object();
                    succJson["name"] = succ_unit->non_kernel_prologue->outlined_func->getName();
                    succJson["edgecost"] = 10;
                    nodeKernelJson["successors"].push_back(succJson);
                }
                nlohmann::json plat = json::object();
                plat["name"] = "cpu";
                plat["nodecost"] = 10;
                plat["runfunc"] = func->getName();
                nodeKernelJson["platforms"].push_back(plat);

                outputDagJson[func->getName()] = nodeKernelJson;
            }
        }
    }
    else /* Non-parallel DAG generation */ {
        for (auto &BB : *main_func) {
            for (BasicBlock::iterator II = BB.begin(); II != BB.end(); ++II) {
                if (auto* CI = dyn_cast<CallInst>(II)) {
                    if (outlined_functions.find(CI->getCalledFunction()->getName()) == outlined_functions.end()) {
                        //errs() << "Just so you know, there was a function call to " << CI->getCalledFunction()->getName() << " that I don't have in my map of extracted functions\n";
                        continue;
                    }

                    knownKernelReplaced = false;
                    //auto *val = CI->getCalledFunction();
                    auto *called_func = CI->getCalledFunction();
                    auto &label = outlined_functions.at(called_func->getName()).second;
                    nlohmann::json nodeJson = json::object();
                    nodeJson["arguments"] = json::array();
                    nodeJson["predecessors"] = json::array();
                    nodeJson["successors"] = json::array();
                    nodeJson["platforms"] = json::array();

                    // Note: the last operand is always a reference to the function being called. We can ignore it
                    for (size_t opNum = 0; opNum < CI->getNumOperands()-1; opNum++) {
                        auto *arg = CI->getOperand(opNum);
                        if (auto* AI = dyn_cast<AllocaInst>(arg)) {
                            if (alloca_map.find(AI) == alloca_map.end()) {
                                errs() << "Encountered an alloca instruction that I don't have in my map. Assuming it's the return value\n";
                                nodeJson["arguments"].push_back("_var_ret");
                            } else {
                                nodeJson["arguments"].push_back(alloca_map[AI]->name);
                            }
                        } else {
                            errs() << "Encountered a function with a non-alloca operand. Function: " << called_func->getName() << ", Operand: ";
                            arg->print(errs());
                            errs() << "\n";
                        }
                    }

                    if (call_idx > 0) {
                        nlohmann::json predJson = json::object();
                        predJson["name"] = "FuncCall_" + to_string(call_idx-1);
                        predJson["edgecost"] = 10;
                        nodeJson["predecessors"].push_back(predJson);
                    }
                    if (CI != lastCallInst) {
                        nlohmann::json succJson = json::object();
                        succJson["name"] = "FuncCall_" + to_string(call_idx+1);
                        succJson["edgecost"] = 10;
                        nodeJson["successors"].push_back(succJson);
                    }
                    nlohmann::json plat = json::object();
                    if (SemanticOpt) {
                        if (label.empty()) {}
                        else if (label == "FFT[1D][2048][complex][float64][forward]") {
                            outs() << "Function " << called_func->getName() << " is labeled as kernel " << label << ". Adding in optimized implementation\n";
                            // Note: temporarily only enabling accelerator optimization and only looking for aarch64 shared object
                            plat["name"] = "cpu";
                            plat["nodecost"] = 10;
                            plat["runfunc"] = called_func->getName();
                            nodeJson["platforms"].push_back(plat);
                            nlohmann::json plat2 = json::object();
                            plat2["name"] = "fft";
                            plat2["nodecost"] = 5;
                            plat2["runfunc"] = "fft2048_accel";
                            plat2["shared_object"] = "fft.so";
                            nodeJson["platforms"].push_back(plat2);
                            knownKernelReplaced = true;
                        } else if (label == "FFT[1D][256][complex][float64][forward]") {
                            outs() << "Function " << called_func->getName() << " is labeled as kernel " << label << ". Adding in optimized implementation\n";
                            plat["name"] = "cpu";
                            plat["nodecost"] = 10;
                            plat["runfunc"] = "fft256_cpu";
                            plat["shared_object"] = "fft.so";
                            nodeJson["platforms"].push_back(plat);
                            nlohmann::json plat2 = json::object();
                            plat2["name"] = "fft";
                            plat2["nodecost"] = 5;
                            plat2["runfunc"] = "fft256_accel";
                            plat2["shared_object"] = "fft.so";
                            nodeJson["platforms"].push_back(plat2);
                            knownKernelReplaced = true;
                        }
                        else {
                            outs() << "Function " << called_func->getName() << " is labeled as kernel " << label << ", but no optimized implementation is available\n";
                        }
                    }
                    if (!knownKernelReplaced) {
                        plat["name"] = "cpu";
                        plat["nodecost"] = 10;
                        plat["runfunc"] = called_func->getName();
                        nodeJson["platforms"].push_back(plat);
                    }
                    if (outputJson["AppName"] == "temporal_mitigation-aarch64" && (called_func->getName() == "main.Node_4" || called_func->getName() == "main.Node_6")) {
                        outs() << "Detected matrix multiply kernel with " << called_func->getName() << ". Adding support for this as well\n";
                        nlohmann::json plat2 = json::object();
                        plat2["name"] = "fft";
                        plat2["nodecost"] = 5;
                        plat2["runfunc"] = "mmult_fpga_kern";
                        plat2["shared_object"] = "mmult-aarch64.so";
                        nodeJson["platforms"].push_back(plat2);
                    }
                    outputDagJson["FuncCall_" + to_string(call_idx)] = nodeJson;
                    call_idx++;
                }
            }
        }
    }

    outputJson["Variables"] = outputVariablesJson;
    outputJson["DAG"] = outputDagJson;

    ofstream ostream(OutputJSONFilename);
    ostream << outputJson.dump(4);
    ostream.close();

    // print human readable module to file
    AssemblyAnnotationWriter *write = new llvm::AssemblyAnnotationWriter();
    std::string str;
    llvm::raw_string_ostream rso(str);
    std::filebuf f0;
    f0.open(OutputLLVMFilename, std::ios::out);
    base_module->print(rso, write);
    std::ostream readableStream(&f0);
    readableStream << str;
    f0.close();
}
