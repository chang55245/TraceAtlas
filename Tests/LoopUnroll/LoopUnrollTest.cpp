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
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
// We know gtest is available because it's specified in the CMakeLists.txt
// The linter may not find it, but it will be available during compilation
#define GTEST_HAS_EXCEPTIONS 0
#include "gtest/gtest.h"
#include <memory>

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

// Main test for loop unrolling
TEST(LoopUnrollRuntime, Latch) {
  printf("before context\n");
  LLVMContext C;

  std::unique_ptr<Module> M = parseIR(
    C,
    R"(define i32 @test(i32* %a, i32* %b, i32* %c, i64 %n) {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc, %while.body ]
  %cmp = icmp slt i64 %i.0, %n
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %i.0
  %0 = load i32, i32* %arrayidx
  %arrayidx1 = getelementptr inbounds i32, i32* %c, i64 %i.0
  %1 = load i32, i32* %arrayidx1
  %mul = mul nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %i.0
  store i32 %mul, i32* %arrayidx2
  %inc = add nsw i64 %i.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond
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
  bool ret = UnrollRuntimeLoopRemainder(L, 4, true, false, false, false, &LI,
                                        &SE, &DT, &AC, PreserveLCSSA);
  EXPECT_FALSE(ret);
  
  // Print the function after transformation
  errs() << "Function after transformation:\n";
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

