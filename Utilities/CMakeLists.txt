
add_executable(JR JR.cpp)
target_link_libraries(JR ${llvm_libs} nlohmann_json nlohmann_json::nlohmann_json AtlasUtil)
target_include_directories(JR SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(JR PRIVATE ${LLVM_DEFINITIONS})
target_compile_options(JR PRIVATE -fno-rtti)

set_target_properties(JR
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)


add_executable(KernelSerial KernelSerial.cpp)
target_link_libraries(KernelSerial ${llvm_libs} nlohmann_json nlohmann_json::nlohmann_json AtlasUtil)
target_include_directories(KernelSerial SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(KernelSerial PRIVATE ${LLVM_DEFINITIONS})
target_compile_options(KernelSerial PRIVATE -fno-rtti)

set_target_properties(KernelSerial
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)


add_executable(PointBasedDAG PointBasedDAG.cpp)
target_link_libraries(PointBasedDAG ${llvm_libs} nlohmann_json nlohmann_json::nlohmann_json AtlasUtil)
target_include_directories(PointBasedDAG SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(PointBasedDAG PRIVATE ${LLVM_DEFINITIONS})
target_compile_options(PointBasedDAG PRIVATE -fno-rtti)

set_target_properties(PointBasedDAG
	PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" 
)