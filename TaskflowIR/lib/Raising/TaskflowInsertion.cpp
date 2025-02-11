#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "Taskflow/TaskflowDialect.h"
#include "Taskflow/TaskflowOps.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Pass/PassManager.h"
#include "Taskflow/Passes/Passes.h"
#include "llvm/Support/JSON.h"
#include <fstream>


namespace mlir {
namespace taskflow {
#define GEN_PASS_DEF_TASKFLOWINSERTION
#define GEN_PASS_DEF_RESOLVETASKDEPENDENCIES
#include "Taskflow/Passes/Passes.h.inc"
} // namespace taskflow
} // namespace mlir

using namespace mlir;
using namespace mlir::taskflow;

namespace {

class TaskflowInsertionPass
    : public taskflow::impl::TaskflowInsertionBase<TaskflowInsertionPass> {
        
private:
    std::vector<int> schedule;
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
  auto *ScheduleJson = jsonConfig.getAsObject()->get("schedule");
  if (!ScheduleJson) {
      getOperation()->emitError() << "ScheduleJson is null" 
                                << llvm::toString(jsonOrError.takeError());
      signalPassFailure();
      return;
    }
    llvm::json::Array *scheduleArrayValue = ScheduleJson->getAsArray();
    for (auto arrValue : *scheduleArrayValue) {
      schedule.push_back(static_cast<int>(*arrValue.getAsInteger()));
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
      if (calleeName.starts_with("taskflow_task")) {
        std::string taskid = calleeName.split('_').second.split('_').second.str();
        int taskid_int = std::stoi(taskid);


        builder.setInsertionPoint(callOp);
        
        // Create graph_start before the first taskflow operation in sequence
        if (schedule.size() > 0 && schedule[1] == taskid_int) {
          auto graphStartOp = builder.create<taskflow::GraphStartOp>(
              callOp.getLoc(), 
              builder.getI32IntegerAttr(graphId));
        }

        // Create task definition
        
        auto taskDefOp = builder.create<taskflow::TaskDefOp>(
            callOp.getLoc(),            
            /*task_handle=*/builder.getType<taskflow::TaskNodeType>(),
            /*dependencies=*/ValueRange{},
            /*node_id=*/builder.getI32IntegerAttr(taskid_int));
        

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
        int schedule_size = schedule.size();
        if (schedule_size > 0 && schedule[schedule_size - 1] == taskid_int) {
          builder.setInsertionPointAfter(taskDefOp);
          builder.create<taskflow::GraphEndOp>(
              callOp.getLoc(),
              builder.getI32IntegerAttr(graphId));
          graphId++;
        }

        // Remove the original call
        callOp.erase();      
      }
      
    });

    // Create a pass manager and run the resolve task dependencies pass
    PassManager pm(&getContext());
    pm.addPass(createResolveTaskDependenciesPass(dagFile));
    if (failed(runPipeline(pm, module))) {
      signalPassFailure();
    }    
  }

};

class ResolveTaskDependenciesPass
    : public taskflow::impl::ResolveTaskDependenciesBase<ResolveTaskDependenciesPass> {
private:
  std::map<int, std::set<int>> dagDependencies;
  void parseDagFile(StringRef dagFile) {
  llvm::errs() << "dependece pass DAG file: " << dagFile << "\n";
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
    dagDependencies[static_cast<int>(*(*arr)[1].getAsInteger())].insert(static_cast<int>(*(*arr)[0].getAsInteger()));

  }
}
public:  
  ResolveTaskDependenciesPass(StringRef dagFile) : taskflow::impl::ResolveTaskDependenciesBase<ResolveTaskDependenciesPass>() {
    this->dagFile = dagFile.str();
  }
  void runOnOperation() override {
    ModuleOp module = cast<ModuleOp>(getOperation());
    OpBuilder builder(&getContext());
    // Read the DAG file
    StringRef tdagFile = this->dagFile; 
    parseDagFile(tdagFile);
    // std::vector<Value> dependencies;
    // module.walk([&](taskflow::TaskDefOp taskDefOp) {
    //   int taskid = taskDefOp.getNodeId();
    //   for (auto dep : dagDependencies[taskid]) {
    //     dependencies.push_back(taskDefOp.getDependencies()[dep]);
    //   }
    //   // taskDefOp.setDependencies(dependencies);
    // });
  }
};
}// namespace
namespace mlir {
namespace taskflow {
std::unique_ptr<Pass> createTaskflowInsertionPass() {
  return std::make_unique<TaskflowInsertionPass>();
}

std::unique_ptr<Pass> createResolveTaskDependenciesPass(StringRef dagFile) {
  return std::make_unique<ResolveTaskDependenciesPass>(dagFile);
}

} // namespace taskflow
} // namespace mlir 


