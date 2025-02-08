#ifndef TASKFLOW_PASSES_TD
#define TASKFLOW_PASSES_TD

include "mlir/Pass/PassBase.td"

def TaskflowInsertion : Pass<"taskflow-insertion"> {
  let summary = "Convert LLVM calls to Taskflow operations";
  let dependentDialects = [
    "LLVM::LLVMDialect",
    "taskflow::TaskflowDialect"
  ];
  let constructor = "mlir::taskflow::createTaskflowInsertionPass()";
}

def TaskflowPatternInsertion : Pass<"taskflow-pattern-insertion"> {
  let summary = "Convert LLVM calls to Taskflow operations using pattern matching";
  let description = [{
    This pass uses MLIR's pattern matching infrastructure to convert LLVM function
    calls to Taskflow operations when the function name starts with "tf_" or "taskflow_".
  }];
  let dependentDialects = [
    "LLVM::LLVMDialect",
    "taskflow::TaskflowDialect"
  ];
  let constructor = "mlir::taskflow::createTaskflowPatternInsertionPass()";
}

#endif // TASKFLOW_PASSES_TD