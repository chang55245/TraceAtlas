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
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      TaskDefOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    
    // Get the first function call from the body
    LLVM::CallOp funcCall;
    for (Operation &bodyOp : op.getBody().front()) {
      if (auto callOp = dyn_cast<LLVM::CallOp>(&bodyOp)) {
        funcCall = callOp;
        break;
      }
    }
    
    if (!funcCall)
      return failure();
    
    // Get the taskflow handle from the parent ApplicationStartOp
    Value tfHandle;
    op->getParentRegion()->walk([&](LLVM::CallOp callOp) {
      if (auto callee = callOp.getCallee()) {
        if (*callee == "taskflow_create") {
          tfHandle = callOp.getResult();
        }
      }
    });
    
    if (!tfHandle)
      return failure();

    // Create task with the function
    SmallVector<Value, 3> args;
    args.push_back(tfHandle);
    
    // Convert function name to LLVM pointer type
    auto funcType = LLVM::LLVMPointerType::get(rewriter.getContext(), 8);
    auto callee = funcCall.getCallee();
    if (!callee) return failure();
    auto symbolRef = FlatSymbolRefAttr::get(rewriter.getContext(), *callee);
    auto globalPtr = rewriter.create<LLVM::AddressOfOp>(loc, funcType, symbolRef);
    
    args.push_back(globalPtr);
    args.push_back(globalPtr);  // Same pointer for both function and data

    auto taskHandle = rewriter.create<LLVM::CallOp>(
        loc,
        LLVM::LLVMPointerType::get(rewriter.getContext(), 8),
        "taskflow_create_task",
        args);
    
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
public:
  using OpConversionPattern::OpConversionPattern;

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
        }
      }
    });
    
    if (!tfHandle)
      return failure();

    // Add dependencies for all TaskDefOps before this GraphEndOp
    Block *parentBlock = op->getBlock();
    for (Operation &blockOp : *parentBlock) {
      if (auto taskOp = dyn_cast<TaskDefOp>(&blockOp)) {
        for (Value dep : taskOp.getDependencies()) {
          rewriter.create<LLVM::CallOp>(
              loc,
              TypeRange{},
              "taskflow_add_dependency",
              ValueRange{taskOp.getResult(), dep});
        }
      }
    }
    
    // Execute the taskflow with the handle
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        "taskflow_execute",
        ValueRange{tfHandle});
    
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

    // Add dependency between patterns
    target.addDynamicallyLegalOp<TaskDefOp>([](TaskDefOp op) {
      // Only allow TaskDefOp to be converted after GraphEndOp is converted
      auto *parentBlock = op->getBlock();
      for (Operation &blockOp : *parentBlock) {
        if (isa<GraphEndOp>(blockOp))
          return false;
      }
      return true;
    });

    // Setup patterns in the correct order
    RewritePatternSet patterns(context);
    patterns.add<ApplicationStartOpLowering>(typeConverter, context);
    patterns.add<GraphEndOpLowering>(typeConverter, context);
    patterns.add<TaskDefOpLowering>(typeConverter, context);
    patterns.add<GraphStartOpLowering>(typeConverter, context);
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