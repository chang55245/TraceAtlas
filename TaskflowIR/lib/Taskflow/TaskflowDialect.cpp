#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"

using namespace mlir;
using namespace mlir::taskflow;

#include "Taskflow/TaskflowOpsDialect.cpp.inc"

void TaskflowDialect::initialize() {
    addOperations<
#define GET_OP_LIST
#include "Taskflow/TaskflowOps.cpp.inc"
    >();
} 