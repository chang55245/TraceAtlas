project(pulse_doppler)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

# Create input directory in build/pulse_doppler folder and copy input files
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input/
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)

# Use the functions for pulse_doppler
add_dag_generation_target(${PROJECT_NAME} "${PROJECT_NAME}.c")
add_task_merging_target(${PROJECT_NAME}) 