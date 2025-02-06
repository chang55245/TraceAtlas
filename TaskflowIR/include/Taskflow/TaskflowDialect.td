#ifndef TASKFLOW_DIALECT
#define TASKFLOW_DIALECT

include "mlir/IR/OpBase.td"

def Taskflow_Dialect : Dialect {
	let name = "taskflow";
	let description = [{}];
	let cppNamespace = "::mlir::taskflow";
}

class Taskflow_Op<string mnemonic, list<Trait> traits = []> : Op<Taskflow_Dialect, mnemonic, traits>;


#endif //TASKFLOW_DIALECT