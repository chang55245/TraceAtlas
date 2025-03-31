cmake_minimum_required(VERSION 3.13.4)
project(benchmarks)

# Set paths for different LLVM versions
set(LLVM_9_PATH "/heorot/lchang21/llvm-release/llvm-9/llvm-9" CACHE PATH "Path to LLVM 9")
set(LLVM_13_PATH "/heorot/lchang21/llvm-release/llvm-13/llvm" CACHE PATH "Path to LLVM 13")
set(LLVM_19_PATH "/heorot/lchang21/llvm-release/llvm-19/llvm-19" CACHE PATH "Path to LLVM 19")

# Set paths for other dependencies
set(CEDR_PATH "/heorot/lchang21/cedr-old/zynq_hardware_emulator" CACHE PATH "Path to CEDR")
set(TRACEATLAS_PATH "/heorot/lchang21/TraceAtlas" CACHE PATH "Path to TraceAtlas")
set(LLVM_19_PASS_PATH "${TRACEATLAS_PATH}/llvm-19-passes" CACHE PATH "Path to LLVM 19 passes")
set(TASKFLOW_LIB_PATH "/heorot/lchang21/taskflow/taskflow/taskflow-lib" CACHE PATH "Path to Taskflow library")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# Build LLVM passes
add_subdirectory(llvm-13-passes)
add_subdirectory(llvm-9-passes)

# Build applications
add_subdirectory(pulse_doppler) 
add_subdirectory(wifi-tx)
add_subdirectory(wifi-rx)
add_subdirectory(SAR)
add_subdirectory(task-bench)
add_subdirectory(edge-detection)