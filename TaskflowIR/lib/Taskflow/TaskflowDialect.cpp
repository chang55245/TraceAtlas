#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "Taskflow/TaskflowTypes.h"

using namespace mlir;
using namespace mlir::taskflow;

#include "Taskflow/TaskflowOpsDialect.cpp.inc"

#define GET_OP_CLASSES
#include "Taskflow/TaskflowOps.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "Taskflow/TaskflowOpsTypes.cpp.inc"

void TaskflowDialect::initialize() {

addTypes<
#define GET_TYPEDEF_LIST
#include "Taskflow/TaskflowOpsTypes.cpp.inc"
      >();
    addOperations<
#define GET_OP_LIST
#include "Taskflow/TaskflowOps.cpp.inc"
    >();
} 