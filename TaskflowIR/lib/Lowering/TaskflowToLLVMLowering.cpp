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

/// Helper: lookup the taskflow handle (created by "taskflow_create")
/// in the entry block of the function containing `op`.
static Value lookupTaskflowHandle(Operation *op) {
  if (auto func = op->getParentOfType<LLVM::LLVMFuncOp>()) {
    for (Operation &opInst : func.getBody().front()) {
      if (auto callOp = dyn_cast<LLVM::CallOp>(opInst)) {
        if (auto callee = callOp.getCallee()) {
          // Use the string value of the symbol.
          if (callee == "taskflow_create")
            return callOp.getResult();
        }
      }
    }
  }
  return nullptr;
}

/// Convert ApplicationStartOp to LLVM calls.
class ApplicationStartOpLowering : public OpConversionPattern<ApplicationStartOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ApplicationStartOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();

    // Get the parent function and insert at its entry block so that
    // the taskflow instance value dominates all later uses.
    auto func = op->getParentOfType<LLVM::LLVMFuncOp>();
    if (!func)
      return failure();
    rewriter.setInsertionPointToStart(&func.getBody().front());

    // Create the taskflow instance.
    auto tfHandle = rewriter.create<LLVM::CallOp>(
        loc, 
        LLVM::LLVMPointerType::get(rewriter.getContext()),
        "taskflow_create",
        ValueRange{});
    
    // Replace the op with the created tfHandle.
    rewriter.replaceOp(op, tfHandle.getResults());
    return success();
  }
};

/// Convert TaskDefOp to LLVM calls.
class TaskDefOpLowering : public OpConversionPattern<TaskDefOp> {
public:
  DenseMap<Operation*, Value> &loweredTaskResults;
  DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies;

  TaskDefOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                      DenseMap<Operation*, Value> &loweredTaskResults,
                      DenseMap<Operation*, SmallVector<Value, 4>> &taskDependencies)
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

    // Get taskflow handle from the function entry block.
    Value tfHandle = lookupTaskflowHandle(op);
    if (!tfHandle)
      return failure();

    auto moduleOp = op->getParentOfType<ModuleOp>();
    auto ptrType = LLVM::LLVMPointerType::get(rewriter.getContext());

    // Get task function name first.
    std::string taskFuncName = "taskflow_task_" + std::to_string(op.getNodeId());
    std::string wrapperFuncName = "taskflow_task_wrapper_" + std::to_string(op.getNodeId());
    llvm::errs() << "Creating wrapper function: " << wrapperFuncName << "\n";

    // Get the parent module
    if (!moduleOp) {
        llvm::errs() << "Failed to find parent ModuleOp\n";
        return failure();
    }


    // Save current insertion point.
    auto savedIP = rewriter.saveInsertionPoint();

    // Move to module scope to create the wrapper function.
    rewriter.setInsertionPointToStart(moduleOp.getBody());

    // Create wrapper function type (TaskArgs* -> void)
    auto voidTy = LLVM::LLVMVoidType::get(rewriter.getContext());
    auto ptrTy = LLVM::LLVMPointerType::get(rewriter.getContext());

    // Define TaskArg struct type with unique name
    auto boolTy = IntegerType::get(rewriter.getContext(), 1);
    std::string taskArgStructName = "TaskArg." + std::to_string(op.getNodeId());
    auto taskArgStructTy = LLVM::LLVMStructType::getNewIdentified(
        rewriter.getContext(), 
        taskArgStructName,
        ArrayRef<Type>{ptrTy, boolTy});

    // Define TaskArgs struct type with unique name
    auto sizeTy = IntegerType::get(rewriter.getContext(), 64);
    auto taskArgPtrTy = LLVM::LLVMPointerType::get(rewriter.getContext());
    std::string taskArgsStructName = "TaskArgs." + std::to_string(op.getNodeId());
    auto taskArgsStructTy = LLVM::LLVMStructType::getNewIdentified(
        rewriter.getContext(), 
        taskArgsStructName,
        ArrayRef<Type>{taskArgPtrTy, sizeTy});

    auto wrapperFuncType = LLVM::LLVMFunctionType::get(voidTy, {ptrTy}, false);

    // Create the functions in the module context
    OpBuilder::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToEnd(moduleOp.getBody());

    // Create wrapper function with unique name
    auto wrapperFunc = rewriter.create<LLVM::LLVMFuncOp>(
        loc, wrapperFuncName, wrapperFuncType, LLVM::Linkage::Internal);
    llvm::errs() << "Created wrapper function:\n" << wrapperFunc << "\n";

    // Create the entry block with the TaskArgs* argument
    auto &entryBlock = *wrapperFunc.addEntryBlock(rewriter);
    Value taskArg = entryBlock.getArgument(0);
    llvm::errs() << "Entry block created with argument:\n" << taskArg << "\n";

    rewriter.setInsertionPointToStart(&entryBlock);

    // Extract arguments from TaskArgs structure
    auto zeroIdx = rewriter.create<LLVM::ConstantOp>(
        loc, rewriter.getI32Type(), rewriter.getI32IntegerAttr(0));
    llvm::errs() << "Created zero index:\n" << zeroIdx << "\n";

    auto oneIdx = rewriter.create<LLVM::ConstantOp>(
        loc, rewriter.getI32Type(), rewriter.getI32IntegerAttr(1));
    llvm::errs() << "Created one index:\n" << oneIdx << "\n";

    // Get args array pointer
    auto argsArrayPtr = rewriter.create<LLVM::GEPOp>(
        loc,
        LLVM::LLVMPointerType::get(rewriter.getContext()),
        taskArgsStructTy,
        taskArg,
        ValueRange{zeroIdx, zeroIdx});
    llvm::errs() << "Created GEP for args array:\n" << argsArrayPtr << "\n";

    auto argsArray = rewriter.create<LLVM::LoadOp>(
        loc, taskArgPtrTy, argsArrayPtr);
    llvm::errs() << "Created load for args array:\n" << argsArray << "\n";

    // Extract arguments and create function call
    SmallVector<Value, 4> extractedArgs;
    for (size_t i = 0; i < funcCall.getNumOperands(); ++i) {
        llvm::errs() << "Processing argument " << i << "\n";
        auto argType = funcCall.getOperand(i).getType();
        llvm::errs() << "Argument type: " << argType << "\n";
        
        auto idx = rewriter.create<LLVM::ConstantOp>(
            loc, rewriter.getI64Type(), rewriter.getI32IntegerAttr(i));
        llvm::errs() << "Created index constant:\n" << idx << "\n";
            
        auto argPtr = rewriter.create<LLVM::GEPOp>(
            loc,
            LLVM::LLVMPointerType::get(rewriter.getContext()),
            taskArgStructTy,
            argsArray,
            ValueRange{idx});
        llvm::errs() << "Created GEP for argument:\n" << argPtr << "\n";

        auto argPtrPtr = rewriter.create<LLVM::GEPOp>(
            loc,
            LLVM::LLVMPointerType::get(rewriter.getContext()),
            taskArgStructTy,
            argPtr,
            ValueRange{zeroIdx, zeroIdx});
        llvm::errs() << "Created GEP for argument pointer:\n" << argPtrPtr << "\n";
            
        auto loadedArg = rewriter.create<LLVM::LoadOp>(
            loc, LLVM::LLVMPointerType::get(rewriter.getContext()), argPtrPtr);
        llvm::errs() << "Created load for argument:\n" << loadedArg << "\n";

        auto castedArg = rewriter.create<LLVM::BitcastOp>(
            loc, argType, loadedArg);
        llvm::errs() << "Created bitcast for argument:\n" << castedArg << "\n";
            
        extractedArgs.push_back(castedArg);
    }

    // // Create the task function body
    // auto &taskBlock = *taskFunc.addEntryBlock(rewriter);
    
    // // Move the original function call to the task function
    // rewriter.setInsertionPointToStart(&taskBlock);
    // rewriter.create<LLVM::CallOp>(
    //     loc,
    //     funcCall.getResultTypes(),
    //     funcCall.getCalleeAttr(),
    //     taskBlock.getArguments());
    // rewriter.create<LLVM::ReturnOp>(loc, ValueRange{});

    // Back in wrapper function, create call to task function
    rewriter.setInsertionPointToEnd(&entryBlock);
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        FlatSymbolRefAttr::get(rewriter.getContext(), taskFuncName),
        extractedArgs);
    rewriter.create<LLVM::ReturnOp>(loc, ValueRange{});

    // Restore original insertion point for the rest of the lowering
    rewriter.restoreInsertionPoint(savedIP);

    // Create task arguments structure
    auto numArgs = rewriter.create<LLVM::ConstantOp>(
        loc, 
        rewriter.getI64Type(),
        rewriter.getI64IntegerAttr(funcCall.getNumOperands()));

    auto taskArgsAlloca = rewriter.create<LLVM::CallOp>(
        loc,
        LLVM::LLVMPointerType::get(rewriter.getContext()),
        "create_task_args",
        ValueRange{numArgs});

    // Initialize the arguments array
    for (size_t i = 0; i < funcCall.getNumOperands(); ++i) {
        auto idx = rewriter.create<LLVM::ConstantOp>(
            loc, 
            rewriter.getI32Type(),
            rewriter.getI32IntegerAttr(i));
            
        auto argPtr = rewriter.create<LLVM::BitcastOp>(
            loc,
            LLVM::LLVMPointerType::get(rewriter.getContext()),
            funcCall.getOperand(i));
            
        rewriter.create<LLVM::CallOp>(
            loc,
            TypeRange{},
            "set_task_arg_ptr",
            ValueRange{taskArgsAlloca.getResult(), idx, argPtr});
    }

    // Create task name global string
    LLVM::GlobalOp funcNameGlobal;
    {
        OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(moduleOp.getBody());
        
        std::string uniqueName = "func_name_" + std::to_string(op.getNodeId());
        auto funcNameStr = StringAttr::get(rewriter.getContext(), taskFuncName);
        
        auto arrayType = LLVM::LLVMArrayType::get(
            IntegerType::get(rewriter.getContext(), 8), 
            funcNameStr.getValue().size());
            
        funcNameGlobal = rewriter.create<LLVM::GlobalOp>(
            moduleOp.getLoc(), 
            arrayType,
            /*isConstant=*/true,
            LLVM::Linkage::Internal,
            uniqueName,
            funcNameStr);
    }
    
    auto funcNameGlobalPtr = rewriter.create<LLVM::AddressOfOp>(loc, funcNameGlobal);
    auto zero = rewriter.create<LLVM::ConstantOp>(
        loc, 
        rewriter.getI64Type(), 
        rewriter.getI64IntegerAttr(0));
    auto funcNamePtr = rewriter.create<LLVM::GEPOp>(
        loc,
        LLVM::LLVMPointerType::get(rewriter.getContext()),
        funcNameGlobal.getType(),
        funcNameGlobalPtr,
        ArrayRef<Value>({zero, zero}));

    // Create the wrapper function reference
    auto wrapperSymbolRef = FlatSymbolRefAttr::get(rewriter.getContext(), wrapperFuncName);
    auto wrapperPtr = rewriter.create<LLVM::AddressOfOp>(loc, ptrType, wrapperSymbolRef);

    // Create the task with all required arguments
    auto taskHandle = rewriter.create<LLVM::CallOp>(
        loc, 
        ptrType,
        "taskflow_create_task",
        ValueRange{
            tfHandle,
            funcNamePtr,
            wrapperPtr,
            taskArgsAlloca.getResult()
        });

    // Record results and update dependencies
    loweredTaskResults[op.getOperation()] = taskHandle.getResult();
    SmallVector<Value, 4> deps;
    for (Value dep : adaptor.getDependencies())
        deps.push_back(dep);
    taskDependencies[op.getOperation()] = deps;

    rewriter.replaceOp(op, taskHandle.getResults());
    return success();
  }
};

/// Convert GraphStartOp to LLVM calls.
class GraphStartOpLowering : public OpConversionPattern<GraphStartOp> {
public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      GraphStartOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    // No code generation needed for GraphStartOp.
    rewriter.eraseOp(op);
    return success();
  }
};

/// Convert GraphEndOp to LLVM calls.
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
    
    // Get taskflow handle from the function entry block.
    Value tfHandle = lookupTaskflowHandle(op);
    if (!tfHandle)
      return failure();

    auto ptrType = LLVM::LLVMPointerType::get(rewriter.getContext());

    // Iterate over each recorded TaskDefOp.
    for (auto &entry : taskDependencies) {
      Operation *origTaskOp = entry.first;
      auto loweredTaskResult = loweredTaskResults.lookup(origTaskOp);
      if (!loweredTaskResult)
        continue;
      auto &deps = entry.second;
      for (Value dep : deps) {
        if (auto depDefOp = dep.getDefiningOp()) {
          if (loweredTaskResults.count(depDefOp))
            dep = loweredTaskResults.lookup(depDefOp);
        }
        auto depLLVMTy = typeConverter.convertType(dep.getType());
        if (!depLLVMTy)
          return failure();
        auto depPtr = rewriter.create<LLVM::BitcastOp>(loc, ptrType, dep);
        rewriter.create<LLVM::CallOp>(
            loc, TypeRange{}, "taskflow_add_dependency",
            ValueRange{depPtr, loweredTaskResult});
      }
    }

    // Execute the taskflow.
    rewriter.create<LLVM::CallOp>(
        loc, TypeRange{}, "taskflow_execute", ValueRange{tfHandle});
    rewriter.eraseOp(op);
    return success();
  }
};

/// Convert TaskYieldOp to LLVM return.
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

/// The actual lowering pass.
class TaskflowToLLVMLoweringPass
    : public taskflow::impl::TaskflowToLLVMBase<TaskflowToLLVMLoweringPass> {
public:
  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *context = &getContext();

    // Set LLVM module attributes.
    if (!module->hasAttr(LLVM::LLVMDialect::getDataLayoutAttrName()))
      module->setAttr(LLVM::LLVMDialect::getDataLayoutAttrName(),
                      StringAttr::get(context, ""));
    if (!module->hasAttr(LLVM::LLVMDialect::getTargetTripleAttrName()))
      module->setAttr(LLVM::LLVMDialect::getTargetTripleAttrName(),
                      StringAttr::get(context, ""));

    // Add taskflow runtime function declarations at the end of module.
    OpBuilder builder(module.getBody(), module.getBody()->end());
    
    // Common types.
    auto PtrTy = LLVM::LLVMPointerType::get(context);
    auto voidTy = LLVM::LLVMVoidType::get(context);
    auto i64Ty = IntegerType::get(context, 64);
    auto i32Ty = IntegerType::get(context, 32);
    
    // Create function types.
    auto createFuncTy = LLVM::LLVMFunctionType::get(PtrTy, {}, false);
    auto createTaskFuncTy = LLVM::LLVMFunctionType::get(PtrTy, {PtrTy, PtrTy, PtrTy, PtrTy}, false);
    auto addDependencyFuncTy = LLVM::LLVMFunctionType::get(voidTy, {PtrTy, PtrTy}, false);
    auto executeFuncTy = LLVM::LLVMFunctionType::get(voidTy, {PtrTy}, false);
    auto createTaskArgsFuncTy = LLVM::LLVMFunctionType::get(PtrTy, {i64Ty}, false);
    auto setTaskArgPtrFuncTy = LLVM::LLVMFunctionType::get(voidTy, {PtrTy, i32Ty, PtrTy}, false);

    // Declare taskflow runtime functions.
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

    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(),
        "create_task_args",
        createTaskArgsFuncTy,
        LLVM::Linkage::External);

    builder.create<LLVM::LLVMFuncOp>(
        module.getLoc(),
        "set_task_arg_ptr",
        setTaskArgPtrFuncTy,
        LLVM::Linkage::External);

    // Convert Taskflow types to LLVM types.
    LLVMTypeConverter typeConverter(context);
    typeConverter.addConversion([&context](taskflow::TaskNodeType type) {
      return LLVM::LLVMPointerType::get(context);
    });
    
    // Setup the conversion target.
    ConversionTarget target(*context);
    target.addLegalDialect<LLVM::LLVMDialect>();
    target.addIllegalDialect<taskflow::TaskflowDialect>();
    
    // Mark LLVM ops as legal.
    target.addLegalOp<ModuleOp>();
    target.addLegalOp<LLVM::LLVMFuncOp>();
    target.addLegalOp<LLVM::CallOp>();
    target.addLegalOp<LLVM::ReturnOp>();
    target.addLegalOp<LLVM::ConstantOp>();

    DenseMap<Operation*, Value> loweredTaskResults;
    DenseMap<Operation*, SmallVector<Value, 4>> taskDependencies;

    // Setup patterns in the correct order.
    RewritePatternSet patterns(context);
    patterns.add<ApplicationStartOpLowering>(typeConverter, context);
    patterns.add<TaskDefOpLowering>(typeConverter, context, loweredTaskResults, taskDependencies);
    patterns.add<GraphStartOpLowering>(typeConverter, context);
    patterns.add<TaskYieldOpLowering>(typeConverter, context);
    patterns.add<GraphEndOpLowering>(typeConverter, context, loweredTaskResults, taskDependencies);

    if (failed(applyPartialConversion(module, target, std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<mlir::Pass> mlir::taskflow::createTaskflowToLLVMLoweringPass() {
  return std::make_unique<TaskflowToLLVMLoweringPass>();
}
