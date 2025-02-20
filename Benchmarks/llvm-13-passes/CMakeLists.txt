cmake_minimum_required(VERSION 3.13.4)
project(llvm13_passes)

# Set LLVM paths
set(LLVM_PATH ${LLVM_13_PATH})

# Add LLVM includes
include_directories(${LLVM_PATH}/include)

# Create OMP parallel pass
add_library(omp-parallel SHARED
    omp-parallel.cpp
)

set_target_properties(omp-parallel PROPERTIES
    COMPILE_FLAGS "-g -shared -fPIC -std=c++17 -Wall -fno-rtti"
    PREFIX ""
)

target_link_libraries(omp-parallel
    PRIVATE
    LLVMCore
    LLVMSupport
    LLVMAnalysis
    LLVMTransformUtils
)

# Create SwapAPIs pass
add_library(SwapAPIs SHARED
    SwapAPIs.cpp
)

set_target_properties(SwapAPIs PROPERTIES
    COMPILE_FLAGS "-g -shared -fPIC -std=c++17 -Wall -fno-rtti"
    PREFIX ""
)

target_link_libraries(SwapAPIs
    PRIVATE
    LLVMCore
    LLVMSupport
    LLVMAnalysis
) 