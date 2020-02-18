#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <llvm/Transforms/Utils/CodeExtractor.h>

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

static int BB_UID = 0;
static int VAR_UID = 0;
static int NODE_UID = 0;

const std::map<uint64_t, std::string> knownKernels = {
    {(uint64_t) 4168845590472268746, "fft256"},
    {(uint64_t) 516142931972860765, "fft256"},
    {(uint64_t) 15922181170781114631, "fft256"},
    {(uint64_t) 16328073116492969661, "fft256"},
    {(uint64_t) 8874214564474425628, "fft256"},
    {(uint64_t) 4236372767260402077, "fft256"}
};

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
            default:
                errs() << "Encountered unknown binary operator " << binOp->getOpcodeName() << " (opcode " << binOp->getOpcode() << ")\n";
        }
    }
    return 0;
}

// TODO: Figure out how to properly restore the name of each piece of the block after the blockStr is computed
uint64_t hashBasicBlocks(std::vector<BasicBlock*> &blocks) {
    std::sort(blocks.begin(), blocks.end());
    vector<string> blockStrings;
    hash<string> hasher;
    uint32_t valueId;
    for (BasicBlock* toConvert : blocks)
    {
        valueId = 0;
        string blockStr;
        vector<Value *> namedVals;
        for (BasicBlock::iterator BI = toConvert->begin(), BE = toConvert->end(); BI != BE; ++BI)
        {
            auto *inst = cast<Instruction>(BI);
            unsigned int ops = inst->getNumOperands();
            for (int i = 0; i < ops; i++)
            {
                Value *op = inst->getOperand(i);
//                op->setName("v_" + to_string(valueId++));
                namedVals.push_back(op);
            }
//            if (std::find(namedVals.begin(), namedVals.end(), inst) == namedVals.end())
//            {
//                inst->setName("v_" + to_string(valueId++));
                namedVals.push_back(inst);
//            }
            std::string str;
            llvm::raw_string_ostream rso(str);
            inst->print(rso);
            blockStr += str + "\n";
        }
//        for (Value *v : namedVals)
//        {
//            v->setName("");
//        }
        namedVals.clear();
        blockStr += "\n";
        blockStrings.push_back(blockStr);
    }

    std::sort(blockStrings.begin(), blockStrings.begin());

    string toHash;
    for (const auto& str : blockStrings)
    {
        toHash += str + "\n";
    }

    return hasher(toHash);
}

cl::opt<std::string> AnnotateFilename("a", cl::desc("Specify original input LLVM with annotated BBs"), cl::value_desc("llvm filename"), cl::Required);
cl::opt<std::string> KernelFilename("k", cl::desc("Specify kernel json"), cl::value_desc("kernel filename"), cl::Required);
cl::opt<std::string> DagFilename("d", cl::desc("Specify DAG json"), cl::value_desc("dag filename"), cl::Required);
cl::opt<std::string> AppName("n", cl::desc("Specify name of generated application"), cl::value_desc("name"), cl::Required);
cl::opt<bool> SemanticOpt("semantic-opt", cl::desc("Enable \"semantic optimization\" of recognized kernels"), cl::Optional);

cl::opt<std::string> OutputLLVMFilename("o", cl::desc("Specify output LLVM"), cl::value_desc("output llvm filename"));
cl::opt<std::string> OutputJSONFilename("o2", cl::desc("Specify output JSON"), cl::value_desc("output json filename"));

int main(int argc, char **argv)
{
    cl::ParseCommandLineOptions(argc, argv);
    LLVMContext context;
    SMDiagnostic smerror;
    std::unique_ptr<Module> annotate_ptr = parseIRFile(AnnotateFilename, smerror, context);
    Module *base_module = annotate_ptr.get();
    Function* main_func = base_module->getFunction("main");

    ifstream kernelIfstream(KernelFilename);
    nlohmann::json kernelJson;
    kernelIfstream >> kernelJson;
    kernelIfstream.close();

    ifstream dagIfstream(DagFilename);
    nlohmann::json dagJson;
    dagIfstream >> dagJson;
    dagIfstream.close();

    map<int, BasicBlock *> base_blockMap;
    for (Module::iterator F = base_module->begin(), E = base_module->end(); F != E; ++F)
    {
        for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
        {
            BasicBlock *b = cast<BasicBlock>(BB);
            base_blockMap[BB_UID++] = b;
        }
    }

    map<string, vector<uint32_t>> kernel_block_map;
    vector<vector<uint32_t>> kernel_blocks;
    for (auto &[key, value] : kernelJson.items())
    {
        string index = key;
        vector<uint32_t> kernel = value;
        kernel_block_map[index] = kernel;
        kernel_blocks.push_back(kernel);
    }
    std::sort(std::begin(kernel_blocks), std::end(kernel_blocks), [](auto &el1, auto &el2) -> bool {
        return el1.at(0) < el2.at(0);
    });

    // Determine which basic blocks were not classified as kernels
    vector<uint32_t> non_kernel_blocks;
    bool is_in_kernel;
    for (uint32_t i = 0; i < BB_UID; i++) {
        is_in_kernel = false;
        for (const auto& kernel_elt : kernel_block_map) {
            auto kern_indices = kernel_elt.second;
            if (std::find(kern_indices.begin(), kern_indices.end(), i) != kern_indices.end()) {
                is_in_kernel = true;
                break;
            }
        }
        if (!is_in_kernel){
            non_kernel_blocks.push_back(i);
        }
    }

//    outs() << "These blocks are not present in any kernel: ";
//    for (auto block : non_kernel_blocks) {
//        outs() << block << " ";
//    }
//    outs() << "\n";

    // Group them into contiguous ranges
    vector<vector<uint32_t>> grouped_blocks;
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

//    outs() << "Grouped into contiguous ranges, they look like this: \n";
//    for (const auto& group : grouped_blocks) {
//        for (auto block : group) {
//            outs() << block << " ";
//        }
//        outs() << "\n";
//    }

    // Interleave the kernels and these blocks
    vector<pair<vector<uint32_t>, bool>> interleaved_groups;
    uint32_t idx1 = 0;
    uint32_t idx2 = 0;
    const vector<uint32_t> poison_pill{std::numeric_limits<uint32_t>::max()};
    while (idx1 < kernel_blocks.size() || idx2 < grouped_blocks.size()) {
        vector<uint32_t> kernel_block = (idx1 < kernel_blocks.size()) ? kernel_blocks.at(idx1) : poison_pill;
        vector<uint32_t> grouped_block = (idx2 < grouped_blocks.size()) ? grouped_blocks.at(idx2) : poison_pill;
        if (kernel_block.at(0) <= grouped_block.at(0)) {
            interleaved_groups.emplace_back(kernel_block, true);
            idx1++;
        } else {
            interleaved_groups.emplace_back(grouped_block, false);
            idx2++;
        }
    }

    // Check if BB 0 is in its own unique region or not
    // If it's not, we need to make it so
    // Its AllocaInsts mess up the rest of the program when it comes time to do code extraction currently
//    if (interleaved_groups.at(0).first.size() > 2) {
//        vector<uint32_t> alloca_block{1};
//        interleaved_groups.at(0).first.erase(interleaved_groups.at(0).first.begin());
//        interleaved_groups.insert(interleaved_groups.begin(), pair<vector<uint32_t>, bool>{alloca_block, false});
//    }
    if (interleaved_groups.at(0).first.size() > 1) {
        vector<uint32_t> alloca_block{0};
        interleaved_groups.at(0).first.erase(interleaved_groups.at(0).first.begin());
        interleaved_groups.insert(interleaved_groups.begin(), pair<vector<uint32_t>, bool>{alloca_block, false});
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
    for (BasicBlock::iterator II = base_blockMap[0]->begin(); II != base_blockMap[0]->end(); ++II) {
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
//    for (Module::iterator F = base_module->begin(); F != base_module->end(); ++F) {
//        for (Function::iterator BB = F->begin(); BB != F->end(); ++BB) {
//            for (BasicBlock::iterator II = BB->begin(); II != BB->end(); II++) {
//
//            }
//        }
//    }

    // Next, iterate and find all calloc/malloc calls
    // Associate the amount they're allocating (assuming it's constant or can be found to be sorta-ish constant) with this variable
    for (BasicBlock::iterator II = base_blockMap[0]->begin(); II != base_blockMap[0]->end(); ++II) {
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
//    errs() << "Here is the baseline dominator tree analysis of main:\n\t";
//    dominatorTree.print(errs());
//    errs() << "\n";

    LoopInfo loopInfo(dominatorTree);
    SmallVector<BasicBlock*, 8> latchVec;
    for (auto *loop : loopInfo.getLoopsInPreorder()) {
        errs() << "I found a loop (loop " << loop->getName() << "):\n";
        loop->getLoopLatches(latchVec);
        errs() << "It has " << latchVec.size() << " loop latches:\n";
        for (auto *latch : latchVec) {
            errs() << "\t" << latch->getName() << "\n";
        }
        latchVec.clear();
    }

    bool changed = false;
    for (auto &group : interleaved_groups) {
        if (group.second) {
            // Don't modify kernels
            continue;
        }
        errs() << "I am processing group with blocks: ";
        for (auto block : group.first) {
            errs() << block << " ";
        }
        errs() << "\n";
        changed = false;
        latchVec.clear();
        Loop* loop;
        for (auto idx : group.first) {
            loop = loopInfo.getLoopFor(base_blockMap[idx]);
            if (loop != nullptr) {
                break;
            }
        }
//        Loop* loop = loopInfo.getLoopFor(base_blockMap[group.first.front()]);
        if (loop != nullptr) {
            loop->getLoopLatches(latchVec);
            BasicBlock* furthestLatch = latchVec.front();
            for (auto *latch : latchVec) {
                if (furthestLatch->getName() < latch->getName()) {
                    furthestLatch = latch;
                }
            }
            auto currentEndBlock = group.first.back();
            auto latchBoxIdx = group.first.back();
            try {
                latchBoxIdx = std::stoi(furthestLatch->getName().str().substr(7));
            } catch(exception &err) {
                errs() << "oh no this block wasn't in the form BB_UID_### :((((\n";
            }
            for (auto idx = currentEndBlock+1; idx <= latchBoxIdx; idx++) {
                changed = true;
                group.first.push_back(idx);
                for (auto &other_group : interleaved_groups) {
                    if (other_group == group) {
                        continue;
                    }
                    for (auto other_idx = other_group.first.begin(), other_end = other_group.first.end(); other_idx != other_end;) {
                        if (*other_idx == idx) {
                            other_group.first.erase(other_idx);
                            break;
                        } else {
                            other_idx++;
                        }
                    }
                }
            }
        }
        if (changed) {
            errs() << "I expanded this group to be blocks: ";
            for (auto block : group.first) {
                errs() << block << " ";
            }
            errs() << "\n";
        }
    }

    // Use the CodeExtractor to extract each snippet into a "node" function
    bool successful;
    map<string, pair<Function*, vector<BasicBlock*>>> outlined_functions;
    deque<Function*> outlined_functions_deque;
    //vector<Function*> outlined_functions;
    for (std::size_t i = 0; i < interleaved_groups.size(); i++) {
        successful = false;
        const auto& group = interleaved_groups.at(i);
        if (group.first.empty() || base_blockMap[group.first.front()]->getParent()->getName() != main_func->getName()) {
            continue;
        }
        vector<BasicBlock*> blocks;
        for (auto idx : group.first) {
            blocks.push_back(base_blockMap[idx]);
        }
        //CodeExtractor CE(ArrayRef<BasicBlock *>(blocks), &dominatorTree, false, nullptr, nullptr, nullptr, false, false, "Node_" + to_string(NODE_UID));
        CodeExtractor CE(ArrayRef<BasicBlock *>(blocks), nullptr, false, nullptr, nullptr, nullptr, false, false, "Node_" + to_string(NODE_UID));
        SetVector<Value *> Inputs, Outputs, Sinks;
        CE.findInputsOutputs(Inputs, Outputs, Sinks);

        outs() << "Attempting to outline blocks ";
        for (auto idx : group.first) {
            outs() << idx << " ";
        }
        outs() << (group.second ? "(kernel)\n" : "(non-kernel)\n");
//        outs() << ", hash: " << hashBasicBlocks(blocks) << ")\n";
        Function *OrigF = blocks.at(0)->getParent();
        if (Function *OutF = CE.extractCodeRegion()) {
            OutF->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
            User *U = *OutF->user_begin();
            CallInst *CI = cast<CallInst>(U);
            CallSite CS(CI);
            CI->setIsNoInline();

            successful = true;
            NODE_UID++;

            outlined_functions[OutF->getName()] = {OutF, blocks};
            outlined_functions_deque.push_back(OutF);
        }
        if (successful) {
            outs() << "Successfully outlined region as " << outlined_functions_deque.back()->getName() << "\n";
        } else {
            outs() << "Failed to extract region\n";
        }
    }

//    uint32_t graph_level = 0;
//    map<uint32_t, vector<Function*>> dag_level_map;
//    dag_level_map[graph_level] = vector<Function*>();
//    while (!outlined_functions_deque.empty()) {
//        Function* func = outlined_functions_deque.front();
//        if (dag_level_map[graph_level].empty()) {
//            dag_level_map[graph_level].push_back(func);
//            outlined_functions_deque.pop_front();
//        }
//        for (auto *otherFunc : dag_level_map[graph_level]) {
//            for (std::size_t thisOpNum = 0; thisOpNum < func->getNumOperands(); thisOpNum++) {
//                for (std::size_t otherOpNum = 0; otherOpNum < otherFunc->getNumOperands(); otherOpNum++) {
//                    if
//                }
//            }
//        }
//    }

    unsigned int call_idx = 0;
    bool hasReturnArg = false;
    bool knownKernelReplaced = false;
    bool kernelsAreParallelizable = false;

//    vector<vector<CallInst*>> callInsts;
//    for (auto & BB : *main_func) {
//        for (BasicBlock::iterator II = BB.begin(); II != BB.end(); ++II) {
//            if (auto* CI = dyn_cast<CallInst>(II)) {
//                if (outlined_functions.find(CI->getCalledFunction()->getName()) == outlined_functions.end()) {
//                    errs() << "Just so you know, there was a function call to " << CI->getCalledFunction()->getName() << " that I don't have in my map of extracted functions\n";
//                    continue;
//                }
//
//
//            }
//        }
//    }

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
    hasReturnArg = false;
    knownKernelReplaced = false;

    for (auto & BB : *main_func) {
        for (BasicBlock::iterator II = BB.begin(); II != BB.end(); ++II) {
            if (auto* CI = dyn_cast<CallInst>(II)) {
                if (outlined_functions.find(CI->getCalledFunction()->getName()) == outlined_functions.end()) {
                    errs() << "Just so you know, there was a function call to " << CI->getCalledFunction()->getName() << " that I don't have in my map of extracted functions\n";
                    continue;
                }

                knownKernelReplaced = false;
                auto *val = CI->getCalledFunction();
                auto &called_blocks = outlined_functions.at(val->getName()).second;
                uint64_t hash = hashBasicBlocks(called_blocks);
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
                            hasReturnArg = true;
                        } else {
                            nodeJson["arguments"].push_back(alloca_map[AI]->name);
                        }
                    } else {
                        errs() << "Encountered a function with a non-alloca operand. Function: " << val->getName() << ", Operand: ";
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
                if (!hasReturnArg) {
                    nlohmann::json succJson = json::object();
                    succJson["name"] = "FuncCall_" + to_string(call_idx+1);
                    succJson["edgecost"] = 10;
                    nodeJson["successors"].push_back(succJson);
                }
                nlohmann::json plat = json::object();
                if (SemanticOpt) {
                    if (knownKernels.find(hash) != knownKernels.end()) {
                        const string& knownKernel = knownKernels.at(hash);
                        outs() << "Recognized " << val->getName() << " as kernel " << knownKernel << ", attempting to swap in optimized implementation\n";
                        if (knownKernel == "fft256") {
                            plat["name"] = "cpu";
                            plat["nodecost"] = 10;
                            plat["runfunc"] = knownKernel + "_cpu";
                            plat["shared_object"] = "fft.so";
                            nodeJson["platforms"].push_back(plat);
                            nlohmann::json plat2 = json::object();
                            plat2["name"] = "fft";
                            plat2["nodecost"] = 5;
                            plat2["runfunc"] = knownKernel + "_accel";
                            plat2["shared_object"] = "fft.so";
                            nodeJson["platforms"].push_back(plat2);
                            knownKernelReplaced = true;
                            outs() << "Successfully augmented JSON with " << knownKernel << " platform invocations\n";
                        } else {
                            errs() << "I recognized this kernel, but I don't know how to optimize it. Falling back to standard processing\n";
                        }
                    } else {
                        errs() << "No known kernel found for call to " << val->getName() << " (hash: " << hash << ")\n";
                    }
                }
                if (!knownKernelReplaced) {
                    plat["name"] = "cpu";
                    plat["nodecost"] = 10;
                    plat["runfunc"] = val->getName();
                    nodeJson["platforms"].push_back(plat);
                }
                outputDagJson["FuncCall_" + to_string(call_idx)] = nodeJson;
                call_idx++;
            }
        }
    }
//    for (inst_iterator I = inst_begin(main_func); I != inst_end(main_func); ++I) {
//
//    }

//    for (size_t i = 0; i < outlined_functions_deque.size(); i++) { //auto &[key, val] : outlined_functions) {
//        auto *val = outlined_functions_deque.at(i);
//
//    }

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
