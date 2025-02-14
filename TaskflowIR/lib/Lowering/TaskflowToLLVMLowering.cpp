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
    
    // Create taskflow instance
    auto tfHandle = rewriter.create<LLVM::CallOp>(
        loc, 
        LLVM::LLVMPointerType::get(rewriter.getContext(), 8),
        "taskflow_create",
        ValueRange{});
    
    rewriter.eraseOp(op);
    return success();
  }
};

// Convert TaskDefOp to LLVM calls
class TaskDefOpLowering : public OpConversionPattern<TaskDefOp> {
public:
  
  DenseMap<Operation*, Value> &loweredTaskResults;
  DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies;

  TaskDefOpLowering(TypeConverter &typeConverter, MLIRContext *context, DenseMap<Operation*, Value> &loweredTaskResults, DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies)
      : OpConversionPattern<TaskDefOp>(typeConverter, context),
        loweredTaskResults(loweredTaskResults),
        taskDependencies(taskDependencies) {}

  LogicalResult matchAndRewrite(TaskDefOp op, OpAdaptor adaptor,
                                ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();

    // Get the first function call from the body.
    LLVM::CallOp funcCall;
    for (Operation &bodyOp : op.getBody().front()) {
      if (auto callOp = dyn_cast<LLVM::CallOp>(&bodyOp)) {
        funcCall = callOp;
        break;
      }
    }
    if (!funcCall)
      return failure();

    // Get the taskflow handle from the parent ApplicationStartOp.
    Value tfHandle;
    op->getParentRegion()->walk([&](LLVM::CallOp callOp) {
      if (auto callee = callOp.getCallee()) {
        if (*callee == "taskflow_create")
          tfHandle = callOp.getResult();
      }
    });
    if (!tfHandle)
      return failure();

    // Create the LLVM call to create a task.
    SmallVector<Value, 3> args;
    args.push_back(tfHandle);
    auto funcType = LLVM::LLVMPointerType::get(rewriter.getContext(), 8);
    auto callee = funcCall.getCallee();
    if (!callee)
      return failure();
    auto symbolRef = FlatSymbolRefAttr::get(rewriter.getContext(), *callee);
    auto globalPtr = rewriter.create<LLVM::AddressOfOp>(loc, funcType, symbolRef);
    args.push_back(globalPtr);
    args.push_back(globalPtr);  // Same pointer for function and data.

    auto taskHandle = rewriter.create<LLVM::CallOp>(
        loc, LLVM::LLVMPointerType::get(rewriter.getContext(), 8),
        "taskflow_create_task", args);

    // **Record the lowered result and the dependency list.**
    loweredTaskResults[op.getOperation()] = taskHandle.getResult();
    SmallVector<Value, 4> deps;
    for (Value dep : op.getDependencies())
      deps.push_back(dep);
    taskDependencies[op.getOperation()] = deps;

    rewriter.replaceOp(op, taskHandle.getResults());
    return success();
  }
};

// Convert GraphStartOp to LLVM calls
class GraphStartOpLowering : public OpConversionPattern<GraphStartOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      GraphStartOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc(); 
    rewriter.eraseOp(op);
    return success();
  }
};

// Convert GraphEndOp to LLVM calls
class GraphEndOpLowering : public OpConversionPattern<GraphEndOp> {
private:
  TypeConverter &typeConverter;
  DenseMap<Operation*, Value> &loweredTaskResults;
  DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies;

public:
    GraphEndOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                     DenseMap<Operation*, Value> &loweredTaskResults,
                     DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies)
      : OpConversionPattern<GraphEndOp>(typeConverter, context),
        typeConverter(typeConverter), loweredTaskResults(loweredTaskResults),
        taskDependencies(taskDependencies) {}

  LogicalResult matchAndRewrite(
      GraphEndOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    
    // Get the taskflow handle from the parent ApplicationStartOp
    Value tfHandle;
    op->getParentRegion()->walk([&](LLVM::CallOp callOp) {
      if (auto callee = callOp.getCallee()) {
        if (*callee == "taskflow_create") {
          tfHandle = callOp.getResult();
          return WalkResult::interrupt();
        }
      }
      return WalkResult::advance();
    });
    
    if (!tfHandle)
      return failure();

    auto ptrType = LLVM::LLVMPointerType::get(rewriter.getContext(), 8);

    // Iterate over each recorded TaskDefOp.
    for (auto &entry : taskDependencies) {
      Operation *origTaskOp = entry.first;
      // Look up the lowered result for this task.
      auto loweredTaskResult = loweredTaskResults.lookup(origTaskOp);
      if (!loweredTaskResult)
        continue; // or return failure if this is unexpected.
      auto &deps = entry.second;
      for (Value dep : deps) {
        // If the dependency comes from a TaskDefOp, use its lowered value.
        if (auto depDefOp = dep.getDefiningOp()) {
          if (loweredTaskResults.count(depDefOp))
            dep = loweredTaskResults.lookup(depDefOp);
        }
        // Optionally, ensure the dependency is LLVM-compatible.
        auto depLLVMTy = typeConverter.convertType(dep.getType());
        if (!depLLVMTy)
          return failure();
        auto depPtr = rewriter.create<LLVM::BitcastOp>(loc, ptrType, dep);
        // Create the call to add the dependency.
        rewriter.create<LLVM::CallOp>(
            loc, TypeRange{}, "taskflow_add_dependency",
            ValueRange{depPtr, loweredTaskResult});
      }
    }

    // Finally, execute the taskflow.
    rewriter.create<LLVM::CallOp>(
        loc, TypeRange{}, "taskflow_execute", ValueRange{tfHandle});
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
    rewriter.eraseOp(op);
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

   
    // Set LLVM module attributes
    if (!module->hasAttr(LLVM::LLVMDialect::getDataLayoutAttrName()))
        module->setAttr(LLVM::LLVMDialect::getDataLayoutAttrName(),
                     StringAttr::get(context, ""));
    if (!module->hasAttr(LLVM::LLVMDialect::getTargetTripleAttrName()))
        module->setAttr(LLVM::LLVMDialect::getTargetTripleAttrName(),
                     StringAttr::get(context, ""));

    // Add taskflow runtime function declarations at the end of module
    OpBuilder builder(module.getBody(), module.getBody()->end());
    
    // Common types
    auto i8PtrTy = LLVM::LLVMPointerType::get(context, 8);
    auto voidTy = LLVM::LLVMVoidType::get(context);
    
    // Create function types
    auto createFuncTy = LLVM::LLVMFunctionType::get(i8PtrTy, {}, false);
    auto createTaskFuncTy = LLVM::LLVMFunctionType::get(i8PtrTy, {i8PtrTy, i8PtrTy, i8PtrTy}, false);
    auto addDependencyFuncTy = LLVM::LLVMFunctionType::get(voidTy, {i8PtrTy, i8PtrTy}, false);
    auto executeFuncTy = LLVM::LLVMFunctionType::get(voidTy, {i8PtrTy}, false);

    // Declare taskflow runtime functions
    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(), 
        "taskflow_create", 
        createFuncTy,
        LLVM::Linkage::External);

    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(), 
        "taskflow_create_task",
        createTaskFuncTy,
        LLVM::Linkage::External);

    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(), 
        "taskflow_add_dependency",
        addDependencyFuncTy,
        LLVM::Linkage::External);

    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(), 
        "taskflow_execute",
        executeFuncTy,
        LLVM::Linkage::External);

    // Convert Taskflow types to LLVM types
    LLVMTypeConverter typeConverter(context);
    typeConverter.addConversion([&context](taskflow::TaskNodeType type) {
      return LLVM::LLVMPointerType::get(context, 8);
    });
    
    // Setup the conversion target
    ConversionTarget target(*context);
    target.addLegalDialect<LLVM::LLVMDialect>();
    target.addIllegalDialect<taskflow::TaskflowDialect>();
    
    // Make sure to mark LLVM operations as legal
    target.addLegalOp<ModuleOp>();
    target.addLegalOp<LLVM::LLVMFuncOp>();
    target.addLegalOp<LLVM::CallOp>();
    target.addLegalOp<LLVM::ReturnOp>();
    target.addLegalOp<LLVM::ConstantOp>();

    DenseMap<Operation*, Value> loweredTaskResults;
    DenseMap<Operation*, SmallVector<Value, 4>> taskDependencies;

    // Setup patterns in the correct order
    RewritePatternSet patterns(context);
    patterns.add<ApplicationStartOpLowering>(typeConverter, context);
    patterns.add<TaskDefOpLowering>(typeConverter, context, loweredTaskResults, taskDependencies);
    
    patterns.add<GraphStartOpLowering>(typeConverter, context);
    patterns.add<TaskYieldOpLowering>(typeConverter, context);
    patterns.add<GraphEndOpLowering>(typeConverter, context, loweredTaskResults, taskDependencies);

    if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
      signalPassFailure();
    }
    
  }
};

} // namespace

std::unique_ptr<mlir::Pass> mlir::taskflow::createTaskflowToLLVMLoweringPass() {
  return std::make_unique<TaskflowToLLVMLoweringPass>();
} 