#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "mlir/Pass/PassRegistry.h"

using namespace mlir;

namespace {
struct TaskflowInsertionPass
    : public PassWrapper<TaskflowInsertionPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TaskflowInsertionPass)

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(&getContext());

    // Insert application_start at the beginning of the module
    builder.setInsertionPointToStart(module.getBody());
    builder.create<taskflow::ApplicationStartOp>(module.getLoc());

    // Create a unique graph ID
    int graphId = 0;

    // Walk through all LLVM function calls
    module.walk([&](LLVM::CallOp callOp) {
      // Get the callee name
      StringRef calleeName = callOp.getCallee().value();
      
      // If the function name starts with "tf_" or "taskflow_", create taskflow operations
      if (calleeName.starts_with("tf_") || calleeName.starts_with("taskflow_")) {
        builder.setInsertionPoint(callOp);
        
        // Create graph_start before the first taskflow operation in sequence
        auto graphStartOp = builder.create<taskflow::GraphStartOp>(
            callOp.getLoc(), 
            builder.getI32IntegerAttr(graphId));

        // Create task definition
        auto taskDefOp = builder.create<taskflow::TaskDefOp>(
            callOp.getLoc(),            
            /*task_handle=*/builder.getType<taskflow::TaskNodeType>(),
            /*dependencies=*/ValueRange{});

        // Move the original call into the task body
        Block *taskBody = new Block();
        taskDefOp.getBody().push_back(taskBody);
        builder.setInsertionPointToStart(taskBody);
        
        // Clone the original call operation into the task body
        Operation *newCall = callOp.clone();
        builder.insert(newCall);
        
        // Add yield operation
        builder.create<taskflow::TaskYieldOp>(callOp.getLoc());

        // Create graph_end after the task
        builder.setInsertionPointAfter(taskDefOp);
        builder.create<taskflow::GraphEndOp>(
            callOp.getLoc(),
            builder.getI32IntegerAttr(graphId));

        // Remove the original call
        callOp.erase();
        
        graphId++;
      }
    });
  }
  StringRef getArgument() const final {
    return "convert-to-taskflow";
  }

  StringRef getDescription() const final {
    return "Convert LLVM calls to Taskflow operations";
  }
};
} // namespace

namespace mlir {
namespace taskflow {
std::unique_ptr<Pass> createTaskflowInsertionPass() {
  return std::make_unique<TaskflowInsertionPass>();
}

} // namespace taskflow
} // namespace mlir 


