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


def executeOp : Taskflow_Op<"execute"> {
	let summary = "execute operation";
	//no argument
	//no result

	// Simply the operation name, nothing else in asm attr-dict `(` `)`  
	let assemblyFormat = [{
		attr-dict  	
  	}];
}


//  inspired from async dialect: https://github.com/llvm/llvm-project/blob/main/mlir/test/mlir-opt/async.mlir
//  https://github.com/llvm/llvm-project/blob/c50f7e9a425bfa4ab8655c79a715c88ed3b1e830/mlir/include/mlir/Dialect/Async/IR/AsyncOps.td#L36

// def taskOp : Taskflow_Op<"task",[SingleBlockImplicitTerminator<"yieldOp">]> {
// 	let summary = "task operation";

// 	//arguments: a code region

// 	//attributes: task_name:string, dependence: tasknode
// 	let arguments = (ins OptionalAttr<Builtin_StringAttr>:$taskName);
// 	// task body a region of blocks
// 	let regions = (region SizedRegion<1>:$region);
// 	//results tasknode type var
// 	let results = (outs TaskNodeType:$outputTask);
// 	let assemblyFormat = [{
// 		$region attr-dict   
//   	}];
// }


// def yieldOp :
//     Taskflow_Op<"yield", [
//       HasParent<"taskOp">, Pure, Terminator, ReturnLike]> {
//   let summary = "terminator for task operation";
//   let description = [{
//     The `taskflow.yield` is a special terminator operation for the block inside
//     `taskflow.task` operation.
//   }];

//   let assemblyFormat = [{
// 		attr-dict  	
//   	}];
// }


def definitionOp : Taskflow_Op<"definition"> {
	let summary = "definition operation";
	// arguments: prev task, next task
	let arguments = (ins AnyMemRef : $taskName, AnyMemRef : $taskFunc);
	let results = (outs AnyMemRef : $definedTask);
	// let assemblyFormat = [{
	// 	`(` $taskName `,` $taskFunc `)` attr-dict  	  
  	// }];
}


def precedeOp : Taskflow_Op<"precede"> {
	let summary = "precede operation";
	// arguments: prev task, next task
	// let arguments = (ins TaskNodeType:$prevTask, TaskNodeType:$nextTask);
	let arguments = (ins AnyMemRef:$prevTask, AnyMemRef:$nextTask);
	//no result
	// let assemblyFormat = [{
	// 	`(` $prevTask `,` $nextTask `)` attr-dict  	  
  	// }];
}
#endif //TASKFLOW_OPS