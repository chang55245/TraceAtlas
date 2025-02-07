#ifndef TASKFLOW_TYPES_TD_
#define TASKFLOW_TYPES_TD_

include "TaskflowDialect.td"
include "mlir/IR/AttrTypeBase.td"

class Taskflow_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<Taskflow_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def TaskNodeType : Taskflow_Type<"TaskNode", "task"> {
  let summary = "Taskflow task node type";
  let description = [{
    Represents a handle to a task node in the taskflow graph.
    Used to establish dependencies between tasks.
  }];
}

#endif  // TASKFLOW_TYPES_TD_