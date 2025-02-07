#ifndef TASKFLOW_PASSES_H
#define TASKFLOW_PASSES_H

#include "mlir/Conversion/LLVMCommon/LoweringOptions.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include <memory>


namespace mlir {
class Pass;

namespace taskflow {
std::unique_ptr<Pass> createTaskflowInsertionPass();
} // namespace taskflow

#define GEN_PASS_REGISTRATION
#include "Taskflow/Passes/Passes.h.inc"

} // namespace mlir

#endif // TASKFLOW_PASSES_H 