#ifndef TASKFLOW_TYPES_TD_
#define TASKFLOW_TYPES_TD_

include "TaskflowDialect.td"
include "mlir/IR/AttrTypeBase.td"

// A base class for all types in this dialect
class Taskflow_Type<string name, string typeMnemonic> : TypeDef<Taskflow_Dialect, name> {
  let mnemonic = typeMnemonic;
}

// def TaskNodeType: Taskflow_Type<"TaskNode", "tasknode"> {
//   let summary = "taskflow task type";

//   let description = [{
//     A type for task in taskflow.
//   }];
// }

#endif  // TASKFLOW_TYPES_TD_