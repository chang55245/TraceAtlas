project(edge-detection)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input/
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)
     
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conv.cpp ${CMAKE_CURRENT_SOURCE_DIR}/track.cpp)

add_dag_generation_target(${PROJECT_NAME} "${SOURCES}")
add_task_merging_target(${PROJECT_NAME})
add_pthread_target(${PROJECT_NAME})
