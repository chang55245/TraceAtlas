#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

using namespace llvm;

int main(int argc, char **argv) {
    // Initialize the module and context
    LLVMContext Context;
    SMDiagnostic Err;
    std::unique_ptr<Module> M = parseIRFile("input.ll", Err, Context);

    if (!M) {
        Err.print(argv[0], errs());
        return 1;
    }

    // Initialize analysis managers
    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;

    // Register analyses
    PassBuilder PB;
    PB.registerFunctionAnalyses(FAM);
    PB.registerModuleAnalyses(MAM);

    // Cross-register analysis managers
    FAM.registerPass([&] { return ModuleAnalysisManagerFunctionProxy(MAM); });
    MAM.registerPass([&] { return FunctionAnalysisManagerModuleProxy(FAM); });

    // Create the pass managers
    FunctionPassManager FPM;
    ModulePassManager MPM;

    // Add passes to the function pass manager
    FPM.addPass(YourFunctionPass());

    // Add passes to the module pass manager
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.addPass(YourModulePass());

    // Run the pass pipeline
    MPM.run(*M, MAM);

    return 0;
}