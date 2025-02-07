module {
  taskflow.application_start
  taskflow.graph_start(0)
  %0 = taskflow.task_def() {
    llvm.call @tf_some_function() : () -> ()
    taskflow.yield
  }
  taskflow.graph_end(0)
  taskflow.graph_start(1)
  %1 = taskflow.task_def() {
    llvm.call @tf_another_function() : () -> ()
    taskflow.yield
  }
  taskflow.graph_end(1)
  llvm.func @tf_some_function()
  llvm.func @tf_another_function()
}