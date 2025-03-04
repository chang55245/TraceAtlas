//===- UnrollLoopTest.cpp - Unit tests for UnrollLoop ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
// We know gtest is available because it's specified in the CMakeLists.txt
// The linter may not find it, but it will be available during compilation
#define GTEST_HAS_EXCEPTIONS 0
#include "gtest/gtest.h"
#include <memory>
#include <set>
#include <functional>

using namespace llvm;

static std::unique_ptr<Module> parseIR(LLVMContext &C, const char *IR) {
  SMDiagnostic Err;
  std::unique_ptr<Module> Mod = parseAssemblyString(IR, Err, C);
  if (!Mod) {
    errs() << "Error parsing IR string:\n";
    Err.print("UnrollLoopTests", errs());
    return nullptr;
  }
  
  // Verify the module to catch any structural issues
  std::string VerifyErrorMsg;
  raw_string_ostream OS(VerifyErrorMsg);
  if (verifyModule(*Mod, &OS)) {
    errs() << "Error verifying module: " << VerifyErrorMsg << "\n";
    return nullptr;
  }
  
  return Mod;
}

// Add this helper function before the test
Value *findLoadFromAlloca(Value *V, std::set<Value*> &Visited) {
  // Prevent infinite recursion
  if (Visited.count(V))
    return nullptr;
  Visited.insert(V);

  // If this is a load instruction, check if it's loading from an alloca
  if (LoadInst *Load = dyn_cast<LoadInst>(V)) {
    if (AllocaInst *Alloca = dyn_cast<AllocaInst>(Load->getPointerOperand())) {
      return Load;
    }
  }

  // Recursively check all operands
  if (Instruction *I = dyn_cast<Instruction>(V)) {
    for (Use &U : I->operands()) {
      if (Value *Result = findLoadFromAlloca(U.get(), Visited))
        return Result;
    }
  }

  return nullptr;
}

// Main test for loop unrolling
TEST(LoopUnrollRuntime, Latch) {
  printf("before context\n");
  LLVMContext C;

  std::unique_ptr<Module> M = parseIR(
    C,
    R"(define i32 @test(i32* %a, i32* %b, i32* %c, i64 %n, i64 %start) {
entry:
  %i = alloca i64
  store i64 %start, i64* %i
  br label %while.cond

while.cond:
  %i.val = load i64, i64* %i
  %cmp = icmp slt i64 %i.val, %n
  br i1 %cmp, label %while.body, label %while.end

while.body:
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %i.val
  %0 = load i32, i32* %arrayidx
  %arrayidx1 = getelementptr inbounds i32, i32* %c, i64 %i.val
  %1 = load i32, i32* %arrayidx1
  %mul = mul nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %i.val
  store i32 %mul, i32* %arrayidx2
  %i.next = add nsw i64 %i.val, 1
  store i64 %i.next, i64* %i
  br label %while.cond

while.end:
  ret i32 0
})"
  );

  ASSERT_TRUE(M) << "Failed to parse IR";
  
  auto *F = M->getFunction("test");
  ASSERT_TRUE(F) << "Function 'test' not found in module";
  
  // Print the function for debugging
  errs() << "Function before transformation:\n";
  F->print(errs());
  errs() << "\n";
  
  DominatorTree DT(*F);
  LoopInfo LI(DT);
  AssumptionCache AC(*F);
  TargetLibraryInfoImpl TLII;
  TargetLibraryInfo TLI(TLII);
  ScalarEvolution SE(*F, TLI, AC, DT, LI);

  Loop *L = *LI.begin();

  ASSERT_TRUE(L) << "No loop found in function";

  bool PreserveLCSSA = L->isRecursivelyLCSSAForm(DT,LI);

  errs() << "Attempting to unroll loop...\n";
  
  // Prepare for peeling by setting up the loop boundary variable
  int peelCount = 4;
  
  // Step 1: Set the loop boundary variable to peel count
  // Get the loop's pre-header block to insert instructions
  BasicBlock *Preheader = L->getLoopPreheader();
  
  // Get the loop condition comparison instruction
  BasicBlock *Header = L->getHeader();
  BranchInst *HeaderBr = dyn_cast<BranchInst>(Header->getTerminator());
  if (!HeaderBr || !HeaderBr->isConditional()) {
    errs() << "Header doesn't have a conditional branch\n";
    return;
  }
  
  ICmpInst *Cmp = dyn_cast<ICmpInst>(HeaderBr->getCondition());
  if (!Cmp) {
    errs() << "Loop condition is not an icmp instruction\n";
    return;
  }
  
  // Get the induction variable and bound
  Value *InductionVar = nullptr;
  Value *BoundValue = nullptr;
  AllocaInst *InductionAlloca = nullptr;
  
  // First try to find the induction variable from the comparison
  if (Cmp) {
    Value *Op0 = Cmp->getOperand(0);
    Value *Op1 = Cmp->getOperand(1);

    std::set<Value*> Visited;
    // Try to find a load from alloca in the first operand
    if (Value *Load = findLoadFromAlloca(Op0, Visited)) {
      InductionVar = Load;
      BoundValue = Op1;
      InductionAlloca = cast<AllocaInst>(cast<LoadInst>(Load)->getPointerOperand());
    } else {
      // Reset visited set for second operand
      Visited.clear();
      // Try the second operand
      if (Value *Load = findLoadFromAlloca(Op1, Visited)) {
        InductionVar = Load;
        BoundValue = Op0;
        InductionAlloca = cast<AllocaInst>(cast<LoadInst>(Load)->getPointerOperand());
      }
    }
  }

  if (!InductionVar) {
    errs() << "Could not find induction variable load instruction\n";
    return;
  }

  // If we found the bound, we can try to set it to initial value + peel count
  if (BoundValue) {
    // Create a builder to insert instructions at the end of the preheader
    IRBuilder<> Builder(Preheader->getTerminator());
    
    // Get the initial value by searching through all blocks that dominate the loop header
    Value *InitialValue = nullptr;
    std::set<BasicBlock*> Visited;
    std::function<bool(BasicBlock*)> FindInitialStore = [&](BasicBlock *BB) -> bool {
      // Skip if we've already visited this block
      if (!Visited.insert(BB).second)
        return false;

      // Search for store instruction in this block
      for (auto &I : *BB) {
        if (StoreInst *Store = dyn_cast<StoreInst>(&I)) {
          if (Store->getPointerOperand() == InductionAlloca) {
            InitialValue = Store->getValueOperand();
            return true;
          }
        }
      }

      // Recursively search through predecessors
      for (BasicBlock *Pred : predecessors(BB)) {
        // Only search through blocks that dominate the loop header
        if (DT.dominates(Pred, Header)) {
          if (FindInitialStore(Pred))
            return true;
        }
      }
      return false;
    };

    // Start search from the loop header
    if (!FindInitialStore(Header)) {
      errs() << "Could not find initial value store in any dominating block\n";
      return;
    }
    
    // Get the type from Cmp's first operand
    Type *BoundType = Cmp->getOperand(0)->getType();
    
    // Create a constant for the peel count with the correct type
    Constant *PeelCountValue = ConstantInt::get(BoundType, peelCount, false);
    
    errs() << "Setting loop boundary to initial value plus peel count: " << peelCount << "\n";
    
    // Convert initial value to the correct type if needed
    Value *InitialValueCasted = Builder.CreateIntCast(InitialValue, BoundType, 
                                                     /*isSigned=*/false, "initial.casted");
    
    // Add the peel count to the initial value
    Value *NewBound = Builder.CreateAdd(InitialValueCasted, PeelCountValue, "new_bound");
    
    // Create a new comparison specifically for the loop condition
    ICmpInst *NewCmp = cast<ICmpInst>(Cmp->clone());
    
    if (Cmp->getOperand(0) == BoundValue) {
      NewCmp->setOperand(0, NewBound);
    } else if (Cmp->getOperand(1) == BoundValue) {
      NewCmp->setOperand(1, NewBound);
    }
    
    // Insert the new comparison instruction before the branch
    NewCmp->insertBefore(HeaderBr);
    
    // Update the branch to use the new comparison
    HeaderBr->setCondition(NewCmp);
    Cmp->eraseFromParent();
    
    errs() << "Modified loop boundary to use initial value plus peel count\n";
  }
  errs() << "Function before peeling:\n";
  F->print(errs());
  errs() << "\n";
  // Step 2: Peel the loop
  bool PeelResult = peelLoop(L, peelCount, &LI, &SE, &DT, &AC, PreserveLCSSA);
  errs() << "Peeling result: " << (PeelResult ? "Success" : "Failure") << "\n";
  
  errs() << "Function after peeling:\n";
  F->print(errs());
  errs() << "\n";

  simplifyLoopAfterUnroll(L, true,&LI, &SE, &DT, &AC);

  errs() << "Function after simplification:\n";
  F->print(errs());
  errs() << "\n";
}

// Add a main function so we can run without GTest if needed
int main(int argc, char **argv) {
    printf("before init\n");
  ::testing::InitGoogleTest(&argc, argv);
  printf("after init\n");
  return RUN_ALL_TESTS();
}

