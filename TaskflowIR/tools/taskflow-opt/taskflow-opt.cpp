#include "mlir/IR/Dialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "Taskflow/Passes/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"


int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
//   mlir::registerAllDialects(registry);
  registry.insert<mlir::LLVM::LLVMDialect>();
  registry.insert<mlir::taskflow::TaskflowDialect>();

  mlir::registerTaskflowPasses();

  // Register the taskflow insertion pass
  

  return mlir::asMainReturnCode(
    mlir::MlirOptMain(argc, argv, "Taskflow dialect driver\n", registry));
} 