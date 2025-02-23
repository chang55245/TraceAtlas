# build

cmake -G "Unix Makefiles" \
    -DMLIR_DIR=/heorot/lchang21/llvm-release/llvm-19/llvm-19/lib/cmake/mlir \
    -Dzstd_DIR=/heorot/lchang21/zstd/install/lib/cmake/zstd \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ..

# test
 ./bin/taskflow-opt ../test/test.mlir
./bin/taskflow-opt -taskflow-insertion ../test/input.mlir


./bin/taskflow-opt --pass-pipeline="builtin.module(taskflow-insertion{dag-file=\"../test/input/task_merging_schedule.json\"})" ../test/input/pulse_doppler_main-extracted.mlir -o ../test/output/pulse_doppler_main-extracted_output.mlir


./bin/taskflow-opt -taskflow-to-llvm ../test/output/pulse_doppler_main-extracted_output.mlir -o ../test/output/pulse_doppler_llvm.mlir

# import llvm ir to llvm dialect
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/mlir-translate \
    -import-llvm ../test/sample-llvm-ir-final.ll \
    -mlir-print-debuginfo \
    -o ../test/sample-llvm-ir-final.mlir

# lower to llvm IR and run
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/mlir-translate \
    --mlir-to-llvmir ../test/sample-llvm-ir-final.mlir \
    -o ../test/sample-llvm-ir-back.ll

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/mlir-translate \
    --mlir-to-llvmir ../test/output/pulse_doppler_llvm.mlir \
    -o ../test/output/pulse_doppler_llvm.ll


# fix me
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang -I /heorot/lchang21/cedr-old/zynq_hardware_emulator/libdash ../test/output/pulse_doppler_llvm.ll /heorot/lchang21/TraceAtlas/TaskflowIR/test/output/pulse_doppler-no-main.bc /heorot/lchang21/TraceAtlas/build/lib/libAtlasBackend.a -o ../test/output/pulse_doppler_llvm -L/heorot/lchang21/taskflow/taskflow/taskflow-lib/build -ltaskflow_lib -lstdc++ -lgsl -lgslcblas -lz

# fix me


/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang -c ../test/sample-llvm-ir-back.ll -o ../test/sample-llvm-ir-back.o

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang ../test/sample-llvm-ir-back.ll -o ../test/sample-llvm-ir-back -L/heorot/lchang21/taskflow/taskflow/taskflow-lib/build -ltaskflow_lib -lstdc++

# debugging 
make VERBOSE=1

# todo

- [ ] add test
- [ ] add example
- [ ] define taskflow dialect
  - [ ] what the final llvm dialect should look like
  - [ ] what are the useful operations
- [ ] add taskflow dialect insertion pass
- [ ] add taskflow dialect to llvm dialect


# taskflow dialect application example

taskflow_application_start() // lower to taskflow library initialization

// one application can have multiple graphs depending on the extractable tasks
tasflow_graph_start(id) // lower to taskflow graph clear and so on
%t1 = tasflow_task_def(//catch the variables){ // task body region }
%t2 = tasflow_task_def(){}
%t3 = tasflow_task_def(){}
%t4 = tasflow_task_def(){precedence = %t2,%t3}{}
tasflow_graph_end(id) // execute and so on

taskflow.application_start

taskflow.graph_start(0)
%t1 = taskflow.task_def {
  // task body
  taskflow.yield
}
%t2 = taskflow.task_def {
  taskflow.yield
}
%t3 = taskflow.task_def {
  taskflow.yield
}
%t4 = taskflow.task_def(%t2, %t3) {
  taskflow.yield
}
taskflow.graph_end(0)