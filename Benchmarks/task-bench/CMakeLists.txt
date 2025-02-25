project(task-bench)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

# Use the functions for task-bench
add_extra_static_libs("/heorot/lchang21/task-bench/core/libcore_s.a")
add_dag_generation_target(${PROJECT_NAME} "${PROJECT_NAME}.cpp")
add_task_merging_target(${PROJECT_NAME}) 