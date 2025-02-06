#ifndef TASKFLOW_OPS
#define TASKFLOW_OPS

include "mlir/IR/OpBase.td"
include "Taskflow/TaskflowDialect.td"

def TaskOp : Op<Taskflow_Dialect, "task"> {
    let summary = "Taskflow task operation";
    let description = [{
        A task operation represents a computational task in the taskflow.
        
        Example:
        ```mlir
        %0 = taskflow.task
        ```
    }];
}

#endif // TASKFLOW_OPS 