#ifndef TASKFLOW_TASKFLOWOPS_H
#define TASKFLOW_TASKFLOWOPS_H


// fix ton of errors by adding these includes
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "Taskflow/TaskflowTypes.h"

#define GET_OP_CLASSES
#include "Taskflow/TaskflowOps.h.inc"

#endif // TASKFLOW_TASKFLOWOPS_H 