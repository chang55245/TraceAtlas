include_guard()

# Common settings and checks
if(NOT DEFINED CEDR_PATH)
    message(FATAL_ERROR "CEDR_PATH is not defined")
endif()

if(NOT DEFINED LLVM_9_PATH)
    message(FATAL_ERROR "LLVM_9_PATH is not defined")
endif()

if(NOT DEFINED TRACEATLAS_PATH)
    message(FATAL_ERROR "TRACEATLAS_PATH is not defined")
endif()

# Add this after the other checks
#if(NOT EXISTS "${CMAKE_BINARY_DIR}/llvm-9-passes/llvm9_passes.so")
#    message(FATAL_ERROR "llvm9_passes.so not found. Please build the LLVM 9 passes first.")
#endif()

# Define paths for Atlas tools and passes
set(ATLAS_TOOLS_PATH "${TRACEATLAS_PATH}/build/bin")
set(ATLAS_PASSES_PATH "${TRACEATLAS_PATH}/build/lib")
set(LLVM9_PASSES_PATH "${CMAKE_BINARY_DIR}/llvm-9-passes")
set(LLVM13_PASSES_PATH "${CMAKE_BINARY_DIR}/llvm-13-passes")
set(TRACEATLAS_PASS_SHARED "${ATLAS_PASSES_PATH}/AtlasPasses.so")
set(TRACEATLAS_PASS_BACKEND_STATIC "${ATLAS_PASSES_PATH}/libAtlasBackend.a")
set(ATLAS_UTILITIES_PATH "${TRACEATLAS_PATH}/Utilities")
set(CEDR_INTERFACE ${CEDR_PATH}/libdash/dash.cpp ${CEDR_PATH}/libdash/kestrel.cpp)
set(INCLUDES -I ${CEDR_PATH}/libdash)

set(TASKFLOW_IR_PATH "${TRACEATLAS_PATH}/TaskflowIR")
set(COMPILER_NAME "clang-9")
function(set_compiler TARGET_NAME)
    if (TARGET_NAME STREQUAL "clang++")
        set(COMPILER_NAME "clang++-9" PARENT_SCOPE)
    elseif (TARGET_NAME STREQUAL "clang")
        set(COMPILER_NAME "clang-9" PARENT_SCOPE)
    endif()
endfunction()

set(Extra_STATIC_LIBS "")

function(add_extra_static_libs TARGET_NAME)
    set(Extra_STATIC_LIBS "${TARGET_NAME}" PARENT_SCOPE)
endfunction()

# Define function for DAG generation
function(add_dag_generation_target TARGET_NAME SOURCE_FILE)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    add_custom_target(${TARGET_NAME}_DAG_generation
        # Initial compilation -fsanitize=address
        COMMAND ${LLVM_9_PATH}/bin/${COMPILER_NAME} -fno-exceptions -g -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto 
                -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm 
                ${INCLUDES} ${CEDR_INTERFACE} ${SOURCE_FILE}
                -o ${OUTPUT_DIR}/${TARGET_NAME}.initial.bc

        # Function inlining and kernel locating
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${LLVM9_PASSES_PATH}/llvm9_passes.so 
                -DynmFunctionInlining ${OUTPUT_DIR}/${TARGET_NAME}.initial.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.inlined.bc
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -KernelLocating ${OUTPUT_DIR}/${TARGET_NAME}.inlined.bc -S 
                 -o ${OUTPUT_DIR}/${TARGET_NAME}.kernel_located.bc
        
        # Generate and run first tracer
        COMMAND ${LLVM_9_PATH}/bin/clang++-9 ${INCLUDES} -fuse-ld=lld 
                ${OUTPUT_DIR}/${TARGET_NAME}.kernel_located.bc 
                -lm -lz -lpthread -lgsl -lgslcblas 
                ${TRACEATLAS_PASS_BACKEND_STATIC} 
                ${Extra_STATIC_LIBS}
                -o ${OUTPUT_DIR}/${TARGET_NAME}.first_tracer
        COMMAND cd ${OUTPUT_DIR} && ./${TARGET_NAME}.first_tracer

        # Loop unrolling and optimization
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -DynmLoopUnroll -lt ${OUTPUT_DIR}/LoopTraceFile.json 
                ${OUTPUT_DIR}/${TARGET_NAME}.kernel_located.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.unrolled.bc
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -mem2reg -sccp 
                ${OUTPUT_DIR}/${TARGET_NAME}.unrolled.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.mem2reg.bc
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -simplifycfg -adce -reg2mem -simplifycfg -sink
                ${OUTPUT_DIR}/${TARGET_NAME}.mem2reg.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.optimized.bc

        # Split kernel and generate second tracer
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -SplitKernExitEnter ${OUTPUT_DIR}/${TARGET_NAME}.optimized.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.split.bc
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -EncodedTrace ${OUTPUT_DIR}/${TARGET_NAME}.split.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc
        COMMAND ${LLVM_9_PATH}/bin/clang++-9 ${INCLUDES} -fuse-ld=lld 
                ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc 
                -lm -lz -lpthread -lgsl -lgslcblas 
                ${TRACEATLAS_PASS_BACKEND_STATIC} 
                ${Extra_STATIC_LIBS}
                -o ${OUTPUT_DIR}/${TARGET_NAME}.second_tracer
        COMMAND cd ${OUTPUT_DIR} && ./${TARGET_NAME}.second_tracer

        # DAG Generation
        COMMAND ${CMAKE_COMMAND} -E env OUTPUT_DIR=${OUTPUT_DIR}
                ${ATLAS_TOOLS_PATH}/JR -i ${OUTPUT_DIR}/raw.trc 
                -b ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.jr.json
        COMMAND ${CMAKE_COMMAND} -E env OUTPUT_DIR=${OUTPUT_DIR}
                ${ATLAS_TOOLS_PATH}/KernelSerial 
                -t ${OUTPUT_DIR}/raw.trc 
                -k ${OUTPUT_DIR}/${TARGET_NAME}.jr.json 
                -b ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.dag.json
        #COMMAND ${CMAKE_COMMAND} -E env OUTPUT_DIR=${OUTPUT_DIR}
        #        python3 ${ATLAS_UTILITIES_PATH}/dag-compare.py 
        #        ${OUTPUT_DIR}/dag_before_merge.json 
        #        ${OUTPUT_DIR}/dag_after_merge.json
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

# Define function for task merging
function(add_task_merging_target TARGET_NAME)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")

    add_custom_target(${TARGET_NAME}_task_merging
        # Task merging and reordering
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -TaskMergingReorder -tm ${OUTPUT_DIR}/task_merging_schedule.json 
                ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.merged.bc
        COMMAND ${LLVM_9_PATH}/bin/clang++-9 -g -lm -lz -lpthread -I ./ 
                -lgsl -lgslcblas ${OUTPUT_DIR}/${TARGET_NAME}.merged.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.merged.native -fuse-ld=lld 
                ${TRACEATLAS_PASS_BACKEND_STATIC}
                ${Extra_STATIC_LIBS}
        COMMAND cd ${OUTPUT_DIR} && ./${TARGET_NAME}.merged.native

        COMMAND ${LLVM_9_PATH}/bin/llvm-as 
                ${OUTPUT_DIR}/${TARGET_NAME}.merged.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.asm.bc
        
        COMMAND ${LLVM_19_PATH}/bin/llvm-dis 
                ${OUTPUT_DIR}/${TARGET_NAME}.asm.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.merged.new.bc

        # Task extraction steps
        COMMAND ${LLVM_19_PATH}/bin/opt 
                -load-pass-plugin=${TRACEATLAS_PATH}/llvm-19-passes/TaskExtraction.so 
                -passes="MergeTaskExtraction" 
                -tm ${OUTPUT_DIR}/task_merging_schedule.json 
                ${OUTPUT_DIR}/${TARGET_NAME}.merged.new.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.extraction.bc

        # Extract main function
        COMMAND ${LLVM_19_PATH}/bin/llvm-extract
                -S
                ${OUTPUT_DIR}/${TARGET_NAME}.extraction.bc 
                -func=main 
                -o ${OUTPUT_DIR}/${TARGET_NAME}-extracted-main.bc

        # Delete main function from original
        COMMAND ${LLVM_19_PATH}/bin/llvm-extract 
                -S 
                --delete 
                ${OUTPUT_DIR}/${TARGET_NAME}.extraction.bc 
                -func=main 
                -o ${OUTPUT_DIR}/${TARGET_NAME}-deleted-main.bc

        # Convert to MLIR
        COMMAND ${LLVM_19_PATH}/bin/mlir-translate
                -import-llvm
                ${OUTPUT_DIR}/${TARGET_NAME}-extracted-main.bc
                -o ${OUTPUT_DIR}/${TARGET_NAME}-extracted-main.mlir

        # Taskflow insertion and optimization
        COMMAND ${TASKFLOW_IR_PATH}/build/bin/taskflow-opt 
                --pass-pipeline=\"builtin.module\(taskflow-insertion\{dag-file=${OUTPUT_DIR}/task_merging_schedule.json\}\)\"
                ${OUTPUT_DIR}/${TARGET_NAME}-extracted-main.mlir
                -o ${OUTPUT_DIR}/${TARGET_NAME}-taskflow.mlir

        # Convert Taskflow IR back to LLVM IR
        COMMAND ${TASKFLOW_IR_PATH}/build/bin/taskflow-opt
                -taskflow-to-llvm
                ${OUTPUT_DIR}/${TARGET_NAME}-taskflow.mlir
                -o ${OUTPUT_DIR}/${TARGET_NAME}-llvm.mlir

        # Convert MLIR to LLVM IR
        COMMAND ${LLVM_19_PATH}/bin/mlir-translate
                --mlir-to-llvmir
                ${OUTPUT_DIR}/${TARGET_NAME}-llvm.mlir
                -o ${OUTPUT_DIR}/${TARGET_NAME}-llvm.ll

        # Final compilation with Taskflow
        COMMAND ${LLVM_19_PATH}/bin/clang 
                -g 
                -I ${CEDR_PATH}/libdash
                ${OUTPUT_DIR}/${TARGET_NAME}-llvm.ll
                ${OUTPUT_DIR}/${TARGET_NAME}-deleted-main.bc
                ${TRACEATLAS_PASS_BACKEND_STATIC}
                ${Extra_STATIC_LIBS}
                -o ${OUTPUT_DIR}/${TARGET_NAME}-taskflow
                -L${TASKFLOW_LIB_PATH}/build
                -ltaskflow_lib
                -lstdc++
                -lgsl
                -lgslcblas
                -lz
                -lm
        COMMAND cd ${OUTPUT_DIR} && ./${TARGET_NAME}-taskflow
        DEPENDS ${TARGET_NAME}_DAG_generation
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()



function(add_pthread_target TARGET_NAME)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")

    add_custom_target(${TARGET_NAME}_pthread

        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} -TestTrans -dg ${OUTPUT_DIR}/${TARGET_NAME}.dag.json ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc -S -o ${OUTPUT_DIR}/${TARGET_NAME}-transbb.bc

        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} -NonKernelOutliner -jr ${OUTPUT_DIR}/${TARGET_NAME}.jr.json -dg ${OUTPUT_DIR}/${TARGET_NAME}.dag.json ${OUTPUT_DIR}/${TARGET_NAME}-transbb.bc -S -o ${OUTPUT_DIR}/${TARGET_NAME}-nkot.bc


        COMMAND ${LLVM_9_PATH}/bin/clang++-9 -g -O1 -S  -flto -emit-llvm ${CEDR_PATH}/libdash/dummy_no_enqueue.cpp -o ${OUTPUT_DIR}/${TARGET_NAME}-dummy.bc

        COMMAND ${LLVM_9_PATH}/bin/llvm-link ${OUTPUT_DIR}/${TARGET_NAME}-nkot.bc ${OUTPUT_DIR}/${TARGET_NAME}-dummy.bc -S -o ${OUTPUT_DIR}/${TARGET_NAME}-linkdm.bc

        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} -SwapNonkernel -dg ${OUTPUT_DIR}/${TARGET_NAME}.dag.json ${OUTPUT_DIR}/${TARGET_NAME}-linkdm.bc -S -o ${OUTPUT_DIR}/${TARGET_NAME}-swapnk.bc

        COMMAND ${LLVM_13_PATH}/bin/opt -enable-new-pm=0 -load ${LLVM13_PASSES_PATH}/llvm13_passes.so -dg ${OUTPUT_DIR}/${TARGET_NAME}.dag.json -jr ${OUTPUT_DIR}/${TARGET_NAME}.jr.json  -erase ${OUTPUT_DIR}/${TARGET_NAME}-swapnk.bc -S -o ${OUTPUT_DIR}/${TARGET_NAME}-erase.bc

        COMMAND ${LLVM_13_PATH}/bin/clang++ -fopenmp=libiomp5 -lm -lz -lpthread -lgsl -lgslcblas ${OUTPUT_DIR}/${TARGET_NAME}-erase.bc -o ${OUTPUT_DIR}/${TARGET_NAME}-pthread.native -fuse-ld=lld ${TRACEATLAS_PASS_BACKEND_STATIC}
        COMMAND LD_PRELOAD=${LLVM_13_PATH}/lib/libomp.so ./${OUTPUT_DIR}/${TARGET_NAME}-pthread.native

        DEPENDS ${TARGET_NAME}_DAG_generation
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()