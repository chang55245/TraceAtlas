module {
  taskflow.application_start
  llvm.func @main() {
    taskflow.graph_start(0)
    %0 = taskflow.task_def() {
      llvm.call @tf_some_function() : () -> ()
      taskflow.yield
    }
    taskflow.graph_end(0)
    llvm.call @unextractable_function_0() : () -> ()
    llvm.call @unextractable_function_1() : () -> ()
    llvm.call @unextractable_function_2() : () -> ()
    taskflow.graph_start(1)
    %1 = taskflow.task_def() {
      llvm.call @tf_another_function() : () -> ()
      taskflow.yield
    }
    taskflow.graph_end(1)
    llvm.return
  }
  llvm.func @tf_some_function()
  llvm.func @tf_another_function()
  llvm.func @unextractable_function_0()
  llvm.func @unextractable_function_1()
  llvm.func @unextractable_function_2()
}