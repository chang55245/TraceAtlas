#include "Taskflow/TaskflowOps.h"
#include "Taskflow/TaskflowDialect.h"

using namespace mlir;
using namespace mlir::taskflow;

#define GET_OP_CLASSES
#include "Taskflow/TaskflowOps.cpp.inc" 