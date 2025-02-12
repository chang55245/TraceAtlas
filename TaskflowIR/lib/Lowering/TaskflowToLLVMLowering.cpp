#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "mlir/Pass/PassRegistry.h"
#include "Taskflow/Passes/Passes.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"

namespace mlir {
namespace taskflow {
#define GEN_PASS_DEF_TASKFLOWTOLLVM
#include "Taskflow/Passes/Passes.h.inc"
} // namespace taskflow
} // namespace mlir

using namespace mlir;
using namespace mlir::taskflow;

namespace {

// Convert ApplicationStartOp to LLVM calls
class ApplicationStartOpLowering : public OpConversionPattern<ApplicationStartOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ApplicationStartOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    
    // Create LLVM function calls for taskflow initialization
    rewriter.create<LLVM::CallOp>(loc, TypeRange{}, "taskflow_init", ValueRange{});
    rewriter.create<LLVM::CallOp>(loc, TypeRange{}, "taskflow_executor_init", ValueRange{});
    
    rewriter.eraseOp(op);
    return success();
  }
};

// Convert TaskDefOp to LLVM calls
class TaskDefOpLowering : public OpConversionPattern<TaskDefOp> {
public:
  explicit TaskDefOpLowering(LLVMTypeConverter &typeConverter, MLIRContext *context)
      : OpConversionPattern<TaskDefOp>(typeConverter, context) {}

  LogicalResult matchAndRewrite(
      TaskDefOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto *context = rewriter.getContext();
    
    // Get LLVM types for task handle
    auto i8PtrTy = LLVM::LLVMPointerType::get(context, 8);
    
    // Create task object
    auto taskHandle = rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{i8PtrTy}, 
        "taskflow_create_task",
        ValueRange{});

    // Set task ID if available
    if (auto nodeId = op.getNodeId()) {
      rewriter.create<LLVM::CallOp>(
          loc,
          TypeRange{},
          ValueRange{taskHandle.getResult(), 
                    rewriter.create<LLVM::ConstantOp>(loc, nodeId)},
                    "taskflow_set_task_id");
    }

    // Handle task body
    Block &body = op.getBody().front();
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        "taskflow_set_task_work",
        ValueRange{taskHandle.getResult()});

    // Move body operations into a new function
    auto moduleOp = op->getParentOfType<ModuleOp>();
    auto funcName = ("task_work_" + std::to_string(taskCounter++));
    auto funcType = LLVM::LLVMFunctionType::get(LLVM::LLVMVoidType::get(context), {});
    
    auto funcOp = rewriter.create<LLVM::LLVMFuncOp>(
        moduleOp.getLoc(), funcName, funcType);
    
    auto *entryBlock = rewriter.createBlock(&funcOp.getBody());
    rewriter.setInsertionPointToStart(entryBlock);
    
    // Clone body operations into the new function
    for (auto &bodyOp : body.without_terminator()) {
      rewriter.clone(bodyOp);
    }
    
    rewriter.create<LLVM::ReturnOp>(loc, ValueRange{});

    // Add dependencies
    for (Value dep : op.getDependencies()) {
      rewriter.create<LLVM::CallOp>(
          loc,
          TypeRange{},
          "taskflow_add_dependency",
          ValueRange{taskHandle.getResult(), dep});
    }

    rewriter.replaceOp(op, taskHandle.getResults());
    return success();
  }

private:
  static int taskCounter;
};

int TaskDefOpLowering::taskCounter = 0;

// Convert GraphStartOp to LLVM calls
class GraphStartOpLowering : public OpConversionPattern<GraphStartOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      GraphStartOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto *context = rewriter.getContext();
    
    // Create new taskflow graph
    auto graphHandle = rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{LLVM::LLVMPointerType::get(context, 8)},
        "taskflow_create_graph",
        ValueRange{});
    
    // Set graph ID
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        "taskflow_set_graph_id",
        ValueRange{graphHandle.getResult(), 
                  rewriter.create<LLVM::ConstantOp>(loc, op.getGraphId())});
    
    rewriter.eraseOp(op);
    return success();
  }
};

// Convert GraphEndOp to LLVM calls
class GraphEndOpLowering : public OpConversionPattern<GraphEndOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      GraphEndOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    
    // Execute graph
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        "taskflow_run_graph",
        ValueRange{});
    
    rewriter.eraseOp(op);
    return success();
  }
};

// Convert TaskYieldOp to LLVM return
class TaskYieldOpLowering : public OpConversionPattern<TaskYieldOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      TaskYieldOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<LLVM::ReturnOp>(op, ValueRange{});
    return success();
  }
};

// The actual lowering pass
class TaskflowToLLVMLoweringPass
    : public taskflow::impl::TaskflowToLLVMBase<TaskflowToLLVMLoweringPass> {
public:
  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *context = &getContext();

    // Convert Taskflow types to LLVM types
    LLVMTypeConverter typeConverter(context);
    typeConverter.addConversion([&context](taskflow::TaskNodeType type) {
      return LLVM::LLVMPointerType::get(context, 8);
    });
    
    // Setup the conversion target
    ConversionTarget target(*context);
    target.addLegalDialect<LLVM::LLVMDialect>();
    target.addIllegalDialect<taskflow::TaskflowDialect>();

    // Setup patterns
    RewritePatternSet patterns(context);
    patterns.add<ApplicationStartOpLowering>(typeConverter, context);
    patterns.add<TaskDefOpLowering>(typeConverter);
    patterns.add<GraphStartOpLowering>(typeConverter, context);
    patterns.add<GraphEndOpLowering>(typeConverter, context);
    patterns.add<TaskYieldOpLowering>(typeConverter, context);

    // Apply the conversion
    if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

std::unique_ptr<mlir::Pass> mlir::taskflow::createTaskflowToLLVMLoweringPass() {
  return std::make_unique<TaskflowToLLVMLoweringPass>();
} 