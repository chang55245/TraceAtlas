llvm.func @main() -> (){
    llvm.call @taskflow_task_1() : ()->()
    llvm.call @unextractable_function_0() : ()->()
    llvm.call @unextractable_function_1() : ()->()
    llvm.call @unextractable_function_2() : ()->()
    llvm.call @taskflow_task_2() : ()->()
    llvm.return
}

llvm.func @taskflow_task_1() -> ()
llvm.func @taskflow_task_2() -> ()
llvm.func @unextractable_function_0() -> ()
llvm.func @unextractable_function_1() -> ()
llvm.func @unextractable_function_2() -> ()
