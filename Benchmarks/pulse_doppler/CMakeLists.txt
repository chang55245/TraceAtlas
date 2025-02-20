cmake_minimum_required(VERSION 3.13.4)
project(pulse_doppler)

# Set paths
set(CEDR_PATH "/heorot/lchang21/cedr-old")
set(LLVM_13_PATH "/heorot/lchang21/llvm-release/llvm-13/llvm")
set(LLVM_19_PATH "/heorot/lchang21/llvm-release/llvm-19/llvm-19")

# Find required packages
find_package(ZLIB REQUIRED)
find_package(GSL REQUIRED)
find_package(OpenMP REQUIRED)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fPIC -DCPU_ONLY -flto")

# Include directories
include_directories(
    ${CEDR_PATH}/libdash
    ${CEDR_PATH}/applications/APIApps/pulse_doppler
    /mnt/nobackup-09/Dash/Sources/include/gsl/
)

# Define the main target
add_executable(${PROJECT_NAME} 
    pulse_doppler.c
)

# LLVM transformation pipeline
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-staged2.bc
    COMMAND clang-9 -O1 -S -flto -emit-llvm ${CEDR_PATH}/libdash/dummy_function.cpp -o dummy.bc
    COMMAND llvm-link ${PROJECT_NAME}-staged2.bc dummy.bc -S -o ${PROJECT_NAME}-trans.bc
    COMMAND ${LLVM_13_PATH}/bin/opt -enable-new-pm=0 
            -load ${OMP_PASS_SHARED} 
            -dg ${PROJECT_NAME}-dag.json 
            -jr ${PROJECT_NAME}-jr.json 
            -SwapAPIs ${PROJECT_NAME}-trans.bc 
            -S -o ${PROJECT_NAME}-swap2.bc
    DEPENDS ${PROJECT_NAME}
)

# Final linking
add_custom_target(${PROJECT_NAME}_final ALL
    COMMAND clang-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC 
            ${PROJECT_NAME}-swap2.bc -o ${PROJECT_NAME}-omp.so 
            ${TRACEATLAS_PASS_BACKEND_STATIC}
    COMMAND ${CEDR_PATH}/build/sub_dag 
            -a ${CEDR_PATH}/applications/APIApps/pulse_doppler/${PROJECT_NAME}-omp.so 
            -n 1 -p 1
    DEPENDS ${PROJECT_NAME}-staged2.bc
)

# Testing targets
add_custom_target(printBB
    COMMAND ${LLVM_13_PATH}/bin/opt -enable-new-pm=0 
            -load ${CMAKE_SOURCE_DIR}/llvm-13-omp-pass/omp-parallel.so 
            -printBB -dg ${PROJECT_NAME}-dag.json 
            -jr ${PROJECT_NAME}-jr.json 
            ${PROJECT_NAME}-test.bc -S -o test.bc
    COMMAND clang-13 -fopenmp=libiomp5 -lm -lz -lpthread 
            -I/mnt/nobackup-09/Dash/Sources/include/gsl/ -I./ 
            -lgsl -lgslcblas test.bc 
            -o ${PROJECT_NAME}-test.native 
            -fuse-ld=lld ${TRACEATLAS_PASS_BACKEND_STATIC}
)

# Task merging target
add_custom_target(task_merging
    COMMAND clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto 
            -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm 
            ${INCLUDES} ${CEDR_INTERFACE} pulse_doppler.c 
            -o ${PROJECT_NAME}-link.bc
    COMMAND opt-9 -load ${FUNTION_INLINING_PASS_SHARED} 
            -DynmFunctionInlining ${PROJECT_NAME}-link.bc 
            -S -o ${PROJECT_NAME}-link.bc
    COMMAND opt-9 -load ${TRACEATLAS_PASS_SHARED} 
            -KernelLocating ${PROJECT_NAME}-link.bc 
            -S -o ${PROJECT_NAME}-link.bc
    COMMAND clang++-9 ${INCLUDES} -fuse-ld=lld ${PROJECT_NAME}-link.bc 
            -lm -lz -lpthread -lgsl -lgslcblas 
            ${TRACEATLAS_PASS_BACKEND_STATIC} 
            -o ${PROJECT_NAME}.tracer
    COMMAND ./${PROJECT_NAME}.tracer
    COMMAND opt-9 -load ${TRACEATLAS_PASS_SHARED} 
            -DynmLoopUnroll -lt LoopTraceFile.json 
            ${PROJECT_NAME}-link.bc -S -o ${PROJECT_NAME}-unroll.bc
) 