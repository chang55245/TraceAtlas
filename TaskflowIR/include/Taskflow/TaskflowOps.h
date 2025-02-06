#ifndef TASKFLOW_TASKFLOWOPS_H
#define TASKFLOW_TASKFLOWOPS_H

#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"

#include "Taskflow/TaskflowTypes.h"

#define GET_OP_CLASSES
#include "Taskflow/TaskflowOps.h.inc"

#endif // TASKFLOW_TASKFLOWOPS_H 