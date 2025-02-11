#ifndef TASKFLOW_PASSES_H
#define TASKFLOW_PASSES_H


#include "mlir/Pass/Pass.h"
#include <memory>


namespace mlir {

namespace taskflow {
#define GEN_PASS_DECL
#include "Taskflow/Passes/Passes.h.inc"
std::unique_ptr<Pass> createTaskflowInsertionPass();
std::unique_ptr<Pass> createTaskflowPatternInsertionPass();
std::unique_ptr<Pass> createResolveTaskDependenciesPass(StringRef dagFile= "test.dag");


#define GEN_PASS_REGISTRATION
#include "Taskflow/Passes/Passes.h.inc"

} // namespace taskflow

} // namespace mlir

#endif // TASKFLOW_PASSES_H 