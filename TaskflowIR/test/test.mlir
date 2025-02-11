// Initialize application
taskflow.application_start

// Start graph
taskflow.graph_start(0)

// Define tasks
%t1 = taskflow.task_def {predecessors = [], node_id =1} :{
  taskflow.yield
}

%t2 = taskflow.task_def {predecessors = [], node_id=2} :{
  taskflow.yield
}

// Task with dependencies
%t3 = taskflow.task_def {predecessors = [%t1, %t2], node_id=3} :{
  taskflow.yield
}

// End graph
taskflow.graph_end(0)