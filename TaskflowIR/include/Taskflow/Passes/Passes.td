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

#endif // TASKFLOW_PASSES_TD