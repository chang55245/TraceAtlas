#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include <llvm-9/llvm/Support/raw_ostream.h>

using namespace llvm;

namespace {


BasicBlock *getBlockByName(Function *F, StringRef name) {
  for (auto &BB : *F)
    if (BB.getName() == name)
      return &BB;
  return nullptr;
}

struct ExtractorTest : public FunctionPass {
  static char ID;
  ExtractorTest() : FunctionPass(ID) { kern_instance_ctr = 0; }
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;

private:
  uint64_t kern_instance_ctr;
};

bool ExtractorTest::runOnFunction(Function &F) {
  bool modified = false;
  Module *M = F.getParent();
  // LLVMContext Ctx;
  // SMDiagnostic Err;
  // std::unique_ptr<Module> M(parseAssemblyString(R"invalid(
    // define i32 @foo(i32 %x, i32 %y, i32 %z) {
    // header:
    //   %0 = icmp ugt i32 %x, %y
    //   br i1 %0, label %body1, label %body2

    // body1:
    //   %1 = add i32 %z, 2
    //   br label %notExtracted

    // body2:
    //   %2 = mul i32 %z, 7
    //   br label %notExtracted

    // notExtracted:
    //   %3 = phi i32 [ %1, %body1 ], [ %2, %body2 ]
    //   %4 = add i32 %3, %x
    //   ret i32 %4
    // }
  // )invalid",
  //                                               Err, Ctx));

  Function *Func = M->getFunction("foo");
  SmallVector<BasicBlock *, 3> Candidates{ getBlockByName(Func, "header"),
                                           getBlockByName(Func, "body1"),
                                           getBlockByName(Func, "body2") };

  CodeExtractor CE(Candidates);
  // EXPECT_TRUE(CE.isEligible());

  Function *Outlined = CE.extractCodeRegion();

  BasicBlock *Exit = getBlockByName(Func, "notExtracted");
  // BasicBlock *ExitSplit = getBlockByName(Outlined, "notExtracted.split");
  llvm::errs()<< *M;
  errs()<<"test 1 \n";
  modified = true;
  return modified;
}
}


bool ExtractorTest::doInitialization(Module &M) {
  bool modified = false;
  errs()<<"test 2 \n";
  return modified;
}

char ExtractorTest::ID = 0;
static RegisterPass<ExtractorTest> Z("ExtractorTest",
                                "test code extractor ",
                                false, false);