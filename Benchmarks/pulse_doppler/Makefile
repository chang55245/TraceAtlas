ARCH ?= x86

LIBDASH_INC ?= ../../../libdash
#LIBDASH_LIB ?= ../../../build_arm/libdash
LIBDASH_LIB ?= ../../../libdash/build_clang


LIBDASH_POS ?= ../../../libdash/build_clang/libdash.a
LIBDASH_BC ?= ../../../libdash/build_clang/dash.a

INCLUDES = -I $(LIBDASH_INC)
LIBS = -L $(LIBDASH_LIB)

APP_NAME = pulse_doppler
SOURCES = pulse_doppler.c

$(APP_NAME)-$(ARCH).so:
	$(CC) $(CFLAGS) $(INCLUDES) -shared -fPIC $(SOURCES) -o $(APP_NAME)-$(ARCH).so


################################
# Changes Related to TraceAtlas 
################################

TRACE_ATLAS_BUILD ?= /heorot/lchang21/TraceAtlas/build
CEDR_PATH = /heorot/lchang21/cedr-old/zynq_hardware_emulator
DASH_ARCHIVES_LIB ?= ../../../dash-archives
GSL_LLVM_LIB ?= -L $(DASH_ARCHIVES_LIB)/release/gsl/lib
GSL_ARCHIVE ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/libgsl.a
GSL_OBJECT_DIR ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/temp

FUNTION_INLINING_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/new_opt_pass/api_swap_pass.so
TRACEATLAS_PASS_SHARED = $(TRACE_ATLAS_BUILD)/lib/AtlasPasses.so
TRACEATLAS_PASS_BACKEND_STATIC = $(TRACE_ATLAS_BUILD)/lib/libAtlasBackend.a
OMP_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so
CEDR_INTERFACE = ../../../libdash/dash.cpp ../../../libdash/kestrel.cpp



standalone: 
	clang-9 -lm -lz -lpthread -DCPU_ONLY -flto -lgsl -lgslcblas $(INCLUDES)  $(CEDR_INTERFACE) $(TRACEATLAS_PASS_BACKEND_STATIC) pulse_doppler.c -o $(APP_NAME)-standalone
	clang-9 -lm -lz -lpthread -DCPU_ONLY -flto -lgsl -lgslcblas -fopenmp=libiomp5 $(INCLUDES)  $(CEDR_INTERFACE) $(TRACEATLAS_PASS_BACKEND_STATIC) test_pulse_doppler_omp.c -o $(APP_NAME)-standalone-omp

standalone-tracer:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) pulse_doppler.c -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer
    
new:
#-------------------compile the application--------------
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) pulse_doppler.c -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json

#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc
# clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	
test_new:	
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc
	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../libdash/dummy_no_enqueue.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-test0.bc

	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/clang++ -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-pthread.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	LD_PRELOAD=/heorot/lchang21/llvm-release/llvm-13/llvm/lib/libomp.so ./$(APP_NAME)-pthread.native


break_new:
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc
	clang-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o pulse_doppler-trans.bc	
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json pulse_doppler-trans.bc -S -o pulse_doppler-swap1.bc

	opt-9 -load ../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs pulse_doppler-swap1.bc -S -o pulse_doppler-swap2.bc
#------------------------Code Generation------------	
	clang-9 -O3 -lm -lz -lpthread -shared -fPIC pulse_doppler-swap2.bc -o pulse_doppler-pthread.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/pulse_doppler/pulse_doppler-pthread.so -n 1 -p 1


old:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) pulse_doppler.c -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem  $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json

#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc
# clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json pulse_doppler-test.bc -S -o $(APP_NAME)-staged1.bc
	clang-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link pulse_doppler-staged1.bc dummy.bc -S -o pulse_doppler-trans.bc
	
	opt-9 -load ../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-trans.bc -S -o pulse_doppler-swap.bc
#------------------------Code Generation------------	
	clang-9 -O3 -lm -lz -lpthread -shared -fPIC pulse_doppler-swap.bc -o pulse_doppler-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/pulse_doppler/pulse_doppler-serial.so -n 1 -p 1

overhead:
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json
	# $(TRACE_ATLAS_BUILD)/bin/PointBasedDAG -t raw.trc -k $(APP_NAME)-jr.json
omppass:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) pulse_doppler.c -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json

#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc

test_omp:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

# test0:
# #-------------------run the binary for testing--------------
# 	# /heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-staged2.bc

	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-test0.bc

	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/clang++ -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-omp.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-omp.native

test_serial:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/clang++ -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test1.bc -o $(APP_NAME)-serial.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	LD_PRELOAD=/heorot/lchang21/llvm-release/llvm-13/llvm/lib/libomp.so ./$(APP_NAME)-serial.native

break_serial:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	clang-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-test1.bc dummy.bc -S -o $(APP_NAME)-trans.bc

	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc

	clang-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC pulse_doppler-swap2.bc -o pulse_doppler-kernel-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/pulse_doppler/pulse_doppler-kernel-serial.so -n 1 -p 1


#/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg pulse_doppler-dag.json -jr pulse_doppler-jr.json test.bc -S -o test.bc

#-------------------run the binary for testing--------------
# clang-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas test.bc -o $(APP_NAME)-test.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native 
break_omp:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc
#------------------------Code Generation------------	
	clang-9 -O1 -S -flto -emit-llvm $(CEDR_PATH)/libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc

	clang-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC pulse_doppler-swap2.bc -o pulse_doppler-omp.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	$(CEDR_PATH)/build/sub_dag -a $(CEDR_PATH)/applications/APIApps/pulse_doppler/pulse_doppler-omp.so -n 1 -p 1


printBB:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/opt -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg pulse_doppler-dag.json -jr pulse_doppler-jr.json  pulse_doppler-test.bc -S -o test.bc
	clang-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas test.bc -o $(APP_NAME)-test.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test.native




task_merging:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) pulse_doppler.c -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem  $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json
	python3 /heorot/lchang21/TraceAtlas/Utilities/dag-compare.py dag_before_merge.json dag_after_merge.json
task_merging_reorder:
#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TaskMergingReorder -tm task_merging_schedule.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-merging.bc
	clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-merging.bc -o $(APP_NAME)-merging.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-merging.native
	

.PHONY: llvm-viz
llvm-viz: llvm-dag-gen
	python3 $(TRACE_ATLAS)/Utilities/DAG-viz.py $(APP_NAME)-dag.json

clean:
	-rm $(APP_NAME)-*.so
	-rm $(APP_NAME)-*.out
	-rm $(APP_NAME)-*.bc
	-rm $(APP_NAME)-*.json
	-rm $(APP_NAME)-*.tracer
	-rm $(APP_NAME)-*.native
	-rm $(APP_NAME)-*.o
	-rm LoopTraceFile.json
	-rm dummy.bc
	-rm raw.trc
