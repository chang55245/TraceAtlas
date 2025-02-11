#ifndef TASKFLOW_OPS
#define TASKFLOW_OPS

include "TaskflowDialect.td"
include "TaskflowTypes.td"


include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ViewLikeInterface.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/IR/BuiltinAttributes.td"

include "mlir/Dialect/LLVMIR/LLVMOpBase.td"
include "mlir/Dialect/LLVMIR/LLVMInterfaces.td"


// Application level operations
def ApplicationStartOp : Taskflow_Op<"application_start"> {
  let summary = "Initialize taskflow application";
  let description = [{
    Initializes the taskflow library and creates a new taskflow application context.
    This operation should be called before any other taskflow operations.
  }];
  
  let assemblyFormat = "attr-dict";
}

def GraphStartOp : Taskflow_Op<"graph_start"> {
  let summary = "Start a new taskflow graph";
  let description = [{
    Creates a new taskflow graph with the given identifier.
    Multiple graphs can exist within one application.
  }];
  
  let arguments = (ins I32Attr:$graph_id);
  let assemblyFormat = "`(` $graph_id `)` attr-dict";
}

def TaskDefOp : Taskflow_Op<"task_def"> {
  let summary = "Define a taskflow task";
  let description = [{
    Defines a new task in the taskflow graph. The task can contain a body region
    and specify precedence constraints with other tasks.
  }];

  // Task body region
  let regions = (region SizedRegion<1>:$body);
  
  // Optional precedence constraints
  let arguments = (ins Variadic<TaskNodeType>:$dependencies,
    I32Attr:$node_id
  );
  
  // Returns a task handle
  let results = (outs TaskNodeType:$task_handle);
  
  // Assembly format
  let assemblyFormat = " `{` `predecessors` `=` `[` $dependencies `]` `,` `node_id` `=` $node_id `}` attr-dict `:` $body";
  // // Declare the custom method
  // let extraClassDeclaration = [{
  //   void setDependencies(std::vector<Value> dependencies);
  // }];
}

def TaskYieldOp : Taskflow_Op<"yield", [
    HasParent<"TaskDefOp">, 
    Pure, 
    Terminator
]> {
  let summary = "Terminator for task definition regions";
  let description = [{
    Terminates a task definition region. This is implicitly added if not present.
  }];
  
  let assemblyFormat = "attr-dict";
}

def GraphEndOp : Taskflow_Op<"graph_end"> {
  let summary = "End and execute a taskflow graph";
  let description = [{
    Finalizes and executes the taskflow graph with the given identifier.
  }];
  
  let arguments = (ins I32Attr:$graph_id);
  let assemblyFormat = "`(` $graph_id `)` attr-dict";
}
#endif //TASKFLOW_OPS