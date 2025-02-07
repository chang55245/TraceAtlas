cmake_minimum_required(VERSION 3.13.4)
# these should be before the project() call
set(CMAKE_CXX_COMPILER "/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang++" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_C_COMPILER "/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang" CACHE STRING "C compiler" FORCE)

project(taskflow-dialect LANGUAGES CXX C)


set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
# may not be needed
add_link_options("-fuse-ld=/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/ld.lld")

# Find MLIR and LLVM
find_package(MLIR REQUIRED CONFIG)
find_package(zstd REQUIRED CONFIG)


message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

# Link against LLVM libraries
link_directories(${LLVM_BUILD_LIBRARY_DIR})

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)