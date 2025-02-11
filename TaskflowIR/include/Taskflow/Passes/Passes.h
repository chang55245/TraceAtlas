#ifndef TASKFLOW_PASSES_H
#define TASKFLOW_PASSES_H


#include "mlir/Pass/Pass.h"
#include <memory>


namespace mlir {

namespace taskflow {
std::unique_ptr<Pass> createTaskflowInsertionPass();
std::unique_ptr<Pass> createTaskflowPatternInsertionPass();
std::unique_ptr<Pass> createResolveTaskDependenciesPass();

} // namespace taskflow
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "Taskflow/Passes/Passes.h.inc"

} // namespace mlir

#endif // TASKFLOW_PASSES_H 