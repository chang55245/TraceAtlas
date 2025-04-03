project(temporal_mitigation)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input/
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/adjoint.cpp ${CMAKE_CURRENT_SOURCE_DIR}/alternateinverse.cpp ${CMAKE_CURRENT_SOURCE_DIR}/determinant.cpp ${CMAKE_CURRENT_SOURCE_DIR}/display.cpp ${CMAKE_CURRENT_SOURCE_DIR}/divide.cpp ${CMAKE_CURRENT_SOURCE_DIR}/getcofactor.cpp ${CMAKE_CURRENT_SOURCE_DIR}/hermitian.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imagpart.cpp ${CMAKE_CURRENT_SOURCE_DIR}/inverse.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mmadd.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mmult.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mmult4.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mmult64.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mmultiply.cpp ${CMAKE_CURRENT_SOURCE_DIR}/msub.cpp ${CMAKE_CURRENT_SOURCE_DIR}/realpart.cpp ${CMAKE_CURRENT_SOURCE_DIR}/scalableinverse.cpp)

add_dag_generation_target(${PROJECT_NAME} "${SOURCES}")
add_task_merging_target(${PROJECT_NAME})
add_pthread_target(${PROJECT_NAME})
