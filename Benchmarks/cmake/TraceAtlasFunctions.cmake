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
if(NOT EXISTS "${CMAKE_BINARY_DIR}/llvm-9-passes/llvm9_passes.so")
    message(FATAL_ERROR "llvm9_passes.so not found. Please build the LLVM 9 passes first.")
endif()

# Define paths for Atlas tools and passes
set(ATLAS_TOOLS_PATH "${TRACEATLAS_PATH}/build/bin")
set(ATLAS_PASSES_PATH "${TRACEATLAS_PATH}/build/lib")
set(LLVM9_PASSES_PATH "${CMAKE_BINARY_DIR}/llvm-9-passes")
set(TRACEATLAS_PASS_SHARED "${ATLAS_PASSES_PATH}/AtlasPasses.so")
set(TRACEATLAS_PASS_BACKEND_STATIC "${ATLAS_PASSES_PATH}/libAtlasBackend.a")
set(ATLAS_UTILITIES_PATH "${TRACEATLAS_PATH}/Utilities")
set(CEDR_INTERFACE ${CEDR_PATH}/libdash/dash.cpp ${CEDR_PATH}/libdash/kestrel.cpp)
set(INCLUDES -I ${CEDR_PATH}/libdash)

# Define function for DAG generation
function(add_dag_generation_target TARGET_NAME SOURCE_FILE)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    add_custom_target(${TARGET_NAME}_DAG_generation
        # Initial compilation
        COMMAND ${LLVM_9_PATH}/bin/clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto 
                -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm 
                ${INCLUDES} ${CEDR_INTERFACE} ${SOURCE_FILE}
                -o ${OUTPUT_DIR}/${TARGET_NAME}.initial.bc

        # Function inlining and kernel locating
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${LLVM9_PASSES_PATH}/llvm9_passes.so 
                -DynmFunctionInlining ${OUTPUT_DIR}/${TARGET_NAME}.initial.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.inlined.bc
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -KernelLocating ${OUTPUT_DIR}/${TARGET_NAME}.inlined.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.kernel_located.bc
        
        # Generate and run first tracer
        COMMAND ${LLVM_9_PATH}/bin/clang++-9 ${INCLUDES} -fuse-ld=lld 
                ${OUTPUT_DIR}/${TARGET_NAME}.kernel_located.bc 
                -lm -lz -lpthread -lgsl -lgslcblas 
                ${TRACEATLAS_PASS_BACKEND_STATIC} 
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
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -simplifycfg -adce -reg2mem 
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
        COMMAND ${CMAKE_COMMAND} -E env OUTPUT_DIR=${OUTPUT_DIR}
                python3 ${ATLAS_UTILITIES_PATH}/dag-compare.py 
                ${OUTPUT_DIR}/dag_before_merge.json 
                ${OUTPUT_DIR}/dag_after_merge.json
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

# Define function for task merging
function(add_task_merging_target TARGET_NAME)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")

    add_custom_target(${TARGET_NAME}_task_merging
        COMMAND ${LLVM_9_PATH}/bin/opt-9 -load ${TRACEATLAS_PASS_SHARED} 
                -TaskMergingReorder -tm ${OUTPUT_DIR}/${TARGET_NAME}.task_merging_schedule.json 
                ${OUTPUT_DIR}/${TARGET_NAME}.encoded.bc 
                -S -o ${OUTPUT_DIR}/${TARGET_NAME}.merged.bc
        COMMAND ${LLVM_9_PATH}/bin/clang-9 -lm -lz -lpthread 
                -I/mnt/nobackup-09/Dash/Sources/include/gsl/ -I ./ 
                -lgsl -lgslcblas ${OUTPUT_DIR}/${TARGET_NAME}.merged.bc 
                -o ${OUTPUT_DIR}/${TARGET_NAME}.merged.native -fuse-ld=lld 
                ${TRACEATLAS_PASS_BACKEND_STATIC}
        COMMAND cd ${OUTPUT_DIR} && ./${TARGET_NAME}.merged.native
        DEPENDS ${TARGET_NAME}_DAG_generation
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction() 