cmake_minimum_required(VERSION 3.13)

project(LoopUnrollTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LLVM_DIR /heorot/lchang21/llvm-release/llvm-9/llvm-9/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Enable debug build with maximum debugging info
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g3 -O0 -fno-omit-frame-pointer -Wall)

# Find GoogleTest if available
find_package(GTest)
if(NOT GTEST_FOUND)
    # If GoogleTest is not found system-wide, provide path to your local installation
    # or download it as a dependency
    set(GTEST_INCLUDE_DIRS "/heorot/lchang21/gtest/install/include")
    set(GTEST_LIBRARIES "/heorot/lchang21/gtest/install/lib/libgtest.a;/heorot/lchang21/gtest/install/lib/libgtest_main.a")
endif()

# Add include directories for LLVM and GTest
include_directories(
    ${LLVM_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

# Add LLVM definitions
add_definitions(${LLVM_DEFINITIONS})

# Create the executable 
add_executable(LoopUnrollTest LoopUnrollTest.cpp)

# LLVM components needed based on the test content
target_link_libraries(LoopUnrollTest
    PRIVATE
    ${GTEST_LIBRARIES}
    LLVMCore
    LLVMIRReader
    LLVMAsmParser
    LLVMSupport
    LLVMAnalysis
    LLVMTransformUtils
    LLVMScalarOpts
    pthread
    dl
)

# Set compiler features
set_target_properties(LoopUnrollTest PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
