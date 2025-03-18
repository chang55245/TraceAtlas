project(task-bench)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)


set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core.cc ${CMAKE_CURRENT_SOURCE_DIR}/core_c.cc ${CMAKE_CURRENT_SOURCE_DIR}/core_kernel.cc ${CMAKE_CURRENT_SOURCE_DIR}/timer.cc ${CMAKE_CURRENT_SOURCE_DIR}/siphash.c ${CMAKE_CURRENT_SOURCE_DIR}/core_random.c ${CMAKE_CURRENT_SOURCE_DIR}/task-bench.cpp)

# Use the functions for task-bench
add_dag_generation_target(${PROJECT_NAME} "${SOURCES}")
add_task_merging_target(${PROJECT_NAME}) 