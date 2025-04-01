project(rf-convergence)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/RF_convergence.c)

# Create input directory in build/SAR folder and copy input files
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/data)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/data)


set_compiler("clang")
add_dag_generation_target(${PROJECT_NAME} "${SOURCES}")
add_task_merging_target(${PROJECT_NAME}) 