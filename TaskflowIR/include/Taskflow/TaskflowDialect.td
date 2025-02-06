#ifndef TASKFLOW_DIALECT
#define TASKFLOW_DIALECT

include "mlir/IR/OpBase.td"

def Taskflow_Dialect : Dialect {
    let name = "taskflow";
    let summary = "A dialect for representing Taskflow computations";
    let description = [{
        This dialect is used for representing Taskflow parallel computations.
    }];
    let cppNamespace = "::mlir::taskflow";
}

#endif // TASKFLOW_DIALECT 