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
  DenseMap<int, Operation*> &IDtoTaskLoweredOp;
  DenseMap<int, SmallVector<int, 4>> &IDtoTaskDependentIDs;

  TaskDefOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                      DenseMap<int, Operation*> &IDtoTaskLoweredOp,
                      DenseMap<int, SmallVector<int, 4>> &IDtoTaskDependentIDs)
      : OpConversionPattern<TaskDefOp>(typeConverter, context),
        IDtoTaskLoweredOp(IDtoTaskLoweredOp),
        IDtoTaskDependentIDs(IDtoTaskDependentIDs) {}

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

    // Create the entry block with the TaskArgs* argument
    auto &entryBlock = *wrapperFunc.addEntryBlock(rewriter);
    Value taskArg = entryBlock.getArgument(0);

    rewriter.setInsertionPointToStart(&entryBlock);

    // Extract arguments from TaskArgs structure
    auto zeroIdx = rewriter.create<LLVM::ConstantOp>(
        loc, rewriter.getI32Type(), rewriter.getI32IntegerAttr(0));

    auto oneIdx = rewriter.create<LLVM::ConstantOp>(
        loc, rewriter.getI32Type(), rewriter.getI32IntegerAttr(1));

    // Get args array pointer
    auto argsArrayPtr = rewriter.create<LLVM::GEPOp>(
        loc,
        LLVM::LLVMPointerType::get(rewriter.getContext()),
        taskArgsStructTy,
        taskArg,
        ValueRange{zeroIdx, zeroIdx});

    auto argsArray = rewriter.create<LLVM::LoadOp>(
        loc, taskArgPtrTy, argsArrayPtr);

    // Extract arguments and create function call
    SmallVector<Value, 4> extractedArgs;
    std::vector<std::pair<Value, Value>> alloca_new_argPtrPtr;
    for (size_t i = 0; i < funcCall.getNumOperands(); ++i) {
        // Value one = rewriter.create<LLVM::ConstantOp>(
        //     loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));
        // auto alloca_new = rewriter.create<LLVM::AllocaOp>(
        //     loc,
        //     funcCall.getOperand(i).getType(),
        //     funcCall.getOperand(i).getType(),
        //     one);
        
        auto idx = rewriter.create<LLVM::ConstantOp>(
            loc, rewriter.getI64Type(), rewriter.getI32IntegerAttr(i));
            
        auto argPtr = rewriter.create<LLVM::GEPOp>(
            loc,
            LLVM::LLVMPointerType::get(rewriter.getContext()),
            taskArgStructTy,
            argsArray,
            ValueRange{idx});

        auto argPtrPtr = rewriter.create<LLVM::GEPOp>(
            loc,
            LLVM::LLVMPointerType::get(rewriter.getContext()),
            taskArgStructTy,
            argPtr,
            ValueRange{zeroIdx, zeroIdx});

        // Load the pointer
        auto loadedArg = rewriter.create<LLVM::LoadOp>(
            loc, LLVM::LLVMPointerType::get(rewriter.getContext()), argPtrPtr);

        auto loadedArgPtr = rewriter.create<LLVM::LoadOp>(
            loc, LLVM::LLVMPointerType::get(rewriter.getContext()), loadedArg);
        // rewriter.create<LLVM::StoreOp>(
        //     loc,
        //     loadedArg,
        //     alloca_new);   
        extractedArgs.push_back(loadedArgPtr);

        // alloca_new_argPtrPtr.push_back(std::make_pair(alloca_new, argPtrPtr));

    }

    // Back in wrapper function, create call to task function

    
    rewriter.create<LLVM::CallOp>(
        loc,
        TypeRange{},
        FlatSymbolRefAttr::get(rewriter.getContext(), taskFuncName),
        extractedArgs);

    // for (auto &pair : alloca_new_argPtrPtr) {
    //     rewriter.create<LLVM::StoreOp>(
    //         loc,
    //         pair.first,
    //         pair.second);   
    // }
    
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
      Value loadedArgPtr = nullptr;

      // Get the defining operation and check if it's a load operation
      if (auto definingOp = funcCall.getOperand(i).getDefiningOp()) {
        if (auto loadOp = dyn_cast<LLVM::LoadOp>(definingOp)) {
          loadedArgPtr = loadOp.getOperand();
        }
      }

      if (!loadedArgPtr) {
        // alloca
        Value one = rewriter.create<LLVM::ConstantOp>(
            loc, rewriter.getI64Type(), rewriter.getI64IntegerAttr(1));
        auto alloca_new = rewriter.create<LLVM::AllocaOp>(
            loc,
            funcCall.getOperand(i).getType(),
            funcCall.getOperand(i).getType(),
            one);
       
        // store
        rewriter.create<LLVM::StoreOp>(
            loc,
            funcCall.getOperand(i),
            alloca_new);
        loadedArgPtr = alloca_new;
      }

      auto idx = rewriter.create<LLVM::ConstantOp>(
          loc, 
          rewriter.getI32Type(),
          rewriter.getI32IntegerAttr(i));
                       
      rewriter.create<LLVM::CallOp>(
          loc,
          TypeRange{},
          "set_task_arg_ptr",
          ValueRange{taskArgsAlloca.getResult(), idx, loadedArgPtr});
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
    auto taskID = op.getNodeId();
    IDtoTaskLoweredOp[taskID] = taskHandle.getOperation();
    for (auto dep : adaptor.getDependentNodeIds()) {
        auto depID = mlir::cast<IntegerAttr>(dep).getInt();
        IDtoTaskDependentIDs[taskID].push_back(depID);
    }
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
  DenseMap<int, Operation*> &IDtoTaskLoweredOp;
  DenseMap<int, SmallVector<int, 4>> &IDtoTaskDependentIDs;

public:
  GraphEndOpLowering(TypeConverter &typeConverter, MLIRContext *context,
                     DenseMap<int, Operation*> &IDtoTaskLoweredOp,
                     DenseMap<int, SmallVector<int, 4>> &IDtoTaskDependentIDs)
      : OpConversionPattern<GraphEndOp>(typeConverter, context),
        typeConverter(typeConverter),
        IDtoTaskLoweredOp(IDtoTaskLoweredOp),
        IDtoTaskDependentIDs(IDtoTaskDependentIDs) {}

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
    for (auto &entry : IDtoTaskDependentIDs) {
      auto taskID = entry.first;
      auto &deps = entry.second;
      auto taskOp = IDtoTaskLoweredOp.lookup(taskID);
      for (auto depID : deps) {
        auto depTaskOp = IDtoTaskLoweredOp.lookup(depID);
        if (!depTaskOp)
        {
          llvm::errs() << "Failed to find dependent task operation for task " << taskID << "\n";
          continue;
        }
        rewriter.create<LLVM::CallOp>(
            loc, TypeRange{}, "taskflow_add_dependency",
            ValueRange{depTaskOp->getResult(0), taskOp->getResult(0)});
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
    DenseMap<int, Operation*> IDtoTaskLoweredOp;
    DenseMap<int, SmallVector<int, 4>> IDtoTaskDependentIDs;

    // Setup patterns in the correct order.
    RewritePatternSet patterns(context);
    patterns.add<ApplicationStartOpLowering>(typeConverter, context);
    patterns.add<TaskDefOpLowering>(typeConverter, context, IDtoTaskLoweredOp, IDtoTaskDependentIDs);
    patterns.add<GraphStartOpLowering>(typeConverter, context);
    patterns.add<TaskYieldOpLowering>(typeConverter, context);
    patterns.add<GraphEndOpLowering>(typeConverter, context, IDtoTaskLoweredOp, IDtoTaskDependentIDs);

    if (failed(applyPartialConversion(module, target, std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<mlir::Pass> mlir::taskflow::createTaskflowToLLVMLoweringPass() {
  return std::make_unique<TaskflowToLLVMLoweringPass>();
}
