ARCH ?= x86

LIBDASH_INC ?= ../../../libdash
#LIBDASH_LIB ?= ../../../build_arm/libdash
LIBDASH_LIB ?= ../../../libdash/build_clang


LIBDASH_POS ?= ../../../libdash/build_clang/libdash.a
LIBDASH_BC ?= ../../../libdash/build_clang/dash.a

INCLUDES = -I $(LIBDASH_INC)
LIBS = -L $(LIBDASH_LIB)

APP_NAME = SAR
SOURCES = SAR.cpp

TRACE_ATLAS_BUILD ?= /home/lchang21/cedr-traceAtlas/TraceAtlas/build
CEDR_PATH = /home/lchang21/cedr-josh/zynq_hardware_emulator

FUNTION_INLINING_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/new_opt_pass/api_swap_pass.so
TRACEATLAS_PASS_SHARED = $(TRACE_ATLAS_BUILD)/lib/AtlasPasses.so
TRACEATLAS_PASS_BACKEND_STATIC = $(TRACE_ATLAS_BUILD)/lib/libAtlasBackend.a
CEDR_INTERFACE = ../../../libdash/dash.cpp ../../../libdash/kestrel.cpp



$(APP_NAME)-$(ARCH).so:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -shared -fPIC $(SOURCES) -o $(APP_NAME)-$(ARCH).so

.PHONY: standalone
standalone:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  ../../../libdash/dash.cpp ../../../libdash/kestrel.cpp $(SOURCES) -o $(APP_NAME)-link.bc

	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o perf-test.native
	./perf-test.native

old:
	clang++-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  ../../../libdash/dash.cpp ../../../libdash/kestrel.cpp $(SOURCES) -o $(APP_NAME)-link.bc
    
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc	
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json
#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc

	clang++-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)

	./$(APP_NAME)-test.native

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -o $(APP_NAME)-staged.bc
	clang++-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged.bc dummy.bc -o $(APP_NAME)-trans.bc		
	opt-9 -load ../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap.bc

	clang++-9 -O3 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap.bc -o $(APP_NAME)-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/APIApps/SAR/SAR-serial.so -n 1 -p 1

new:
#-------------------compile the application--------------
	clang++-9 -g -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) $(SOURCES) -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	#opt-9 -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json

#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc

# clang++-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
test_new:
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc	

	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc
	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-test0.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-test0.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test0.native
	
break_new:	
	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc

	# opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-swap-test.bc

	# clang++-13 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-swap1.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	# ./$(APP_NAME)-test.native
	#./$(APP_NAME)-test.native > test.txt

	opt-9 -load ../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-swap2.bc
#------------------------Code Generation------------	
	clang++-9 -O3 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-pthread.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/APIApps/SAR/SAR-pthread.so -n 1 -p 1

omppass:
	clang++-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) $(SOURCES) -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9  -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SplitKernExitEnter $(APP_NAME)-unroll.bc -o $(APP_NAME)-opt.bc
	# opt-9 -mem2reg $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -EncodedTrace $(APP_NAME)-opt.bc -S -o $(APP_NAME)-opt.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-opt.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME)-1.tracer
	./$(APP_NAME)-1.tracer
#-------------------DAG Generation--------------
	$(TRACE_ATLAS_BUILD)/bin/JR -i raw.trc -b $(APP_NAME)-opt.bc -o $(APP_NAME)-jr.json
	$(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json

#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TestTrans -dg $(APP_NAME)-dag.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-test.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	
# test1:
# clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native	
# break1:
#-------------------create openMP parallel regions--------------
test_omp:
	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

# test0:
# #-------------------run the binary for testing--------------
# 	# opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-staged2.bc

	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-test0.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-test0.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test0.native

test_serial:
	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test1.bc -o $(APP_NAME)-test1.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test1.native


break_omp:
#------------------------Code Generation------------	
	clang++-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	
	
	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc


	clang++-13 -O3 -fopenmp=libiomp5 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-omp.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/APIApps/SAR/SAR-omp.so -n 1 -p 1

clean:
	-rm $(APP_NAME)-*.so
	-rm $(APP_NAME)-*.out
