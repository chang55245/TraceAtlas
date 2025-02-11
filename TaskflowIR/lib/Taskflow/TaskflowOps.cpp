#include "Taskflow/TaskflowOps.h"
#include "Taskflow/TaskflowDialect.h"

using namespace mlir;
using namespace mlir::taskflow;


void setDependencies(taskflow::TaskDefOp taskDefOp, std::vector<Value> dependencies) {
   taskDefOp.getOperation()->setOperands(dependencies);
}

#define GET_OP_CLASSES
#include "Taskflow/TaskflowOps.cpp.inc" 


