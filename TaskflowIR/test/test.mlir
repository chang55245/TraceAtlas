// Initialize application
taskflow.application_start

// Start graph
taskflow.graph_start(0)

// Define tasks
%t1 = taskflow.task_def() {
  taskflow.yield
}

%t2 = taskflow.task_def() {
  taskflow.yield
}

// Task with dependencies
%t3 = taskflow.task_def(%t1, %t2) {
  taskflow.yield
}

// End graph
taskflow.graph_end(0)