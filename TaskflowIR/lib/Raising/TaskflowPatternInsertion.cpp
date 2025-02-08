#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "mlir/Pass/PassRegistry.h"
#include "Taskflow/Passes/Passes.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace taskflow {
#define GEN_PASS_DEF_TASKFLOWPATTERNINSERTION
#include "Taskflow/Passes/Passes.h.inc"
} // namespace taskflow
} // namespace mlir


using namespace mlir;
using namespace mlir::taskflow;

namespace {

// Pattern to match LLVM calls and convert them to Taskflow operations
class ConvertLLVMCallToTaskflow : public OpRewritePattern<LLVM::CallOp> {
public:
  ConvertLLVMCallToTaskflow(MLIRContext *context, int &graphCounter)
      : OpRewritePattern<LLVM::CallOp>(context), graphId(graphCounter) {}

  LogicalResult matchAndRewrite(LLVM::CallOp callOp,
                              PatternRewriter &rewriter) const override {
    // Check if this is a taskflow function call
    StringRef calleeName = callOp.getCallee().value();
    if (!calleeName.starts_with("tf_") && !calleeName.starts_with("taskflow_"))
      return failure();
   
    if (isa<taskflow::TaskDefOp>(callOp->getParentOp()))
      return failure();

    // Create graph_start operation
    rewriter.create<taskflow::GraphStartOp>(
        callOp.getLoc(),
        rewriter.getI32IntegerAttr(graphId));

    
    // Create task definition
    auto taskDefOp = rewriter.create<taskflow::TaskDefOp>(
        callOp.getLoc(),
        /*task_handle=*/rewriter.getType<taskflow::TaskNodeType>(),
        /*dependencies=*/ValueRange{});

    // Create the task body
    Block *taskBody = new Block();
    taskDefOp.getBody().push_back(taskBody);
    rewriter.setInsertionPointToStart(taskBody);

    // Clone the original call into the task body
    Operation *newCall = callOp.clone();
    rewriter.insert(newCall);

    // Add yield operation
    rewriter.create<taskflow::TaskYieldOp>(callOp.getLoc());

    // Reset insertion point after the task
    rewriter.setInsertionPointAfter(taskDefOp);

    // Create graph_end operation
    rewriter.create<taskflow::GraphEndOp>(
        callOp.getLoc(),
        rewriter.getI32IntegerAttr(graphId));

    // Increment graph ID
    graphId++;

    // Remove the original call
    rewriter.eraseOp(callOp);

    return success();
  }

private:
  int &graphId;
};

void populateTaskflowPatternInsertionPatterns(RewritePatternSet &patterns, int &graphId) {
  patterns.add<ConvertLLVMCallToTaskflow>(patterns.getContext(), graphId);
}

// The actual pass that uses the pattern
class TaskflowPatternInsertionPass
    : public taskflow::impl::TaskflowPatternInsertionBase<TaskflowPatternInsertionPass> {

  void runOnOperation() override {
    ModuleOp module = cast<ModuleOp>(getOperation());
    MLIRContext *context = &getContext();

    // Insert application_start at the beginning
    OpBuilder builder(context);
    builder.setInsertionPointToStart(module.getBody());
    builder.create<taskflow::ApplicationStartOp>(module.getLoc());

    // Create the pattern list
    RewritePatternSet patterns(context);
    populateTaskflowPatternInsertionPatterns(patterns, graphId);

    // Apply the patterns
    if (failed(applyPatternsAndFoldGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
  private:
  int graphId = 0;

};

} // namespace

namespace mlir {
namespace taskflow {

std::unique_ptr<Pass> createTaskflowPatternInsertionPass() {
  return std::make_unique<TaskflowPatternInsertionPass>();
}

} // namespace taskflow
} // namespace mlir 