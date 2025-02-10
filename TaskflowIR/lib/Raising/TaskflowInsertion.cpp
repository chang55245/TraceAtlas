#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "mlir/Pass/PassRegistry.h"
#include "Taskflow/Passes/Passes.h"
#include "llvm/Support/JSON.h"
#include <fstream>

namespace mlir {
namespace taskflow {
#define GEN_PASS_DEF_TASKFLOWINSERTION
#include "Taskflow/Passes/Passes.h.inc"
} // namespace taskflow
} // namespace mlir

using namespace mlir;
using namespace mlir::taskflow;

namespace {

class TaskflowInsertionPass
    : public taskflow::impl::TaskflowInsertionBase<TaskflowInsertionPass> {
        
private:

void parseDagFile(StringRef dagFile) {
  llvm::errs() << "DAG file: " << dagFile << "\n";
    std::ifstream fileStream(dagFile.str());
    if (!fileStream.is_open()) {
      getOperation()->emitError() << "Could not open dagFile JSON file: " << dagFile;
      signalPassFailure();
      return;
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    std::string jsonText = buffer.str();

    auto jsonOrError = llvm::json::parse(jsonText);
    if (!jsonOrError) {
      getOperation()->emitError() << "JSON parsing error: " 
                                << llvm::toString(jsonOrError.takeError());
      signalPassFailure();
      return;
    }
  llvm::json::Value jsonConfig = std::move(*jsonOrError);
  auto *DagEdgeJson = jsonConfig.getAsObject()->get("DAGEdge");
  if (!DagEdgeJson) {
      getOperation()->emitError() << "DagEdgeJson is null" 
                                << llvm::toString(jsonOrError.takeError());
      signalPassFailure();
      return;
    }
  // Now you can use jsonConfig to control your transformation.
  llvm::json::Array *arrayValue = DagEdgeJson->getAsArray();
  for (auto arrValue : *arrayValue) {
    llvm::json::Array *arr = arrValue.getAsArray();
    if (arr->size() != 2) {
      getOperation()->emitError() << "Expected an array with exactly two elements";
      signalPassFailure();
      return;
    }
    if (auto first = (*arr)[0].getAsInteger()) {
      int first_element = static_cast<int>(*first);
      llvm::errs() << "First element: " << first_element << "\n";
    } else {
      getOperation()->emitError() << "First element is not an integer";
      signalPassFailure();
      return;
    }
    // Extract the second element as an integer
    if (auto second = (*arr)[1].getAsInteger()) {
      int second_element = static_cast<int>(*second);
      llvm::errs() << "Second element: " << second_element << "\n";
    } else {
        getOperation()->emitError() << "Second element is not an integer";
        signalPassFailure();
        return;
      }
    }
  }
public:
  TaskflowInsertionPass() = default;  
  void runOnOperation() override {
    ModuleOp module = cast<ModuleOp>(getOperation());
    OpBuilder builder(&getContext());
    // Read the DAG file
    StringRef dagFile = this->dagFile; 
    parseDagFile(dagFile);
    
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

};
} // namespace

namespace mlir {
namespace taskflow {
std::unique_ptr<Pass> createTaskflowInsertionPass() {
  return std::make_unique<TaskflowInsertionPass>();
}

} // namespace taskflow
} // namespace mlir 


