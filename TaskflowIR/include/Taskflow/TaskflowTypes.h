#ifndef TASKFLOW_TASKFLOWTYPES_H
#define TASKFLOW_TASKFLOWTYPES_H

#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"

#define GET_TYPEDEF_CLASSES
#include "Taskflow/TaskflowOpsTypes.h.inc"

#endif // TASKFLOW_TASKFLOWTYPES_H 