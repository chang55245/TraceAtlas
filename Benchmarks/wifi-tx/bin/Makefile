ARCH ?= x86

LIBDASH_INC ?= ../../../../libdash
LIBDASH_LIB ?= ../../../../build_arm/libdash

INCLUDES = -I $(LIBDASH_INC)
LIBS = -L $(LIBDASH_LIB)

APP_NAME = wifi-tx

C_SOURCES = ../src/crc.c ../src/txData.c ../src/baseband_lib.c ../src/viterbi.c \
	../src/interleaver_deintleaver.c ../src/scrambler_descrambler.c ../src/qpsk_Mod_Demod.c \
	../src/CyclicPrefix.c ../src/Preamble_ST_LG.c ../src/diag.c ../src/rfInf.c ../src/pilot.c
CPP_SOURCES = ../src/TX.cpp ../src/fft_hs.cpp
CPP_SOURCES_OMP = ../src/TX-omp.cpp ../src/fft_hs.cpp

SOURCES = $(C_SOURCES) $(CPP_SOURCES)
SOURCESOMP = $(C_SOURCES) $(CPP_SOURCES_OMP)

TRACE_ATLAS_BUILD ?= /home/liangliangdev/traceAtlas/TraceAtlas/build
CEDR_PATH = /home/liangliangdev/cedr/zynq_hardware_emulator
DASH_ARCHIVES_LIB ?= ../../../dash-archives
GSL_LLVM_LIB ?= -L $(DASH_ARCHIVES_LIB)/release/gsl/lib
GSL_ARCHIVE ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/libgsl.a
GSL_OBJECT_DIR ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/temp

FUNTION_INLINING_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/new_opt_pass/api_swap_pass.so
TRACEATLAS_PASS_SHARED = $(TRACE_ATLAS_BUILD)/lib/AtlasPasses.so
TRACEATLAS_PASS_BACKEND_STATIC = $(TRACE_ATLAS_BUILD)/lib/libAtlasBackend.a
OMP_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so
CEDR_INTERFACE = $(CEDR_PATH)/libdash/dash.cpp $(CEDR_PATH)/libdash/kestrel.cpp


$(APP_NAME)-$(ARCH).so:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -shared -fPIC $(SOURCES) -o $(APP_NAME)-$(ARCH).so

standalone: 
	clang++-9 -lm -lz -lpthread -DCPU_ONLY -flto -lgsl -lgslcblas -fopenmp=libiomp5 $(INCLUDES)  ../../../../libdash/dash.cpp ../../../../libdash/kestrel.cpp $(TRACEATLAS_PASS_BACKEND_STATIC)  $(SOURCES) -o $(APP_NAME)-standalone
	clang++-9 -lm -lz -lpthread -DCPU_ONLY -flto -lgsl -lgslcblas -fopenmp=libiomp5 $(INCLUDES)  ../../../../libdash/dash.cpp ../../../../libdash/kestrel.cpp $(TRACEATLAS_PASS_BACKEND_STATIC)  $(SOURCESOMP) -o $(APP_NAME)-standalone-omp


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
	clang++-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test.native

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc


test_new:
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc	
	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../../libdash/dummy_no_enqueue.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-test0.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-test-pthread.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test-pthread.native

# test3:
# 	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

# 	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test1.bc -o $(APP_NAME)-test1.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
# 	./$(APP_NAME)-test1.native


break_new:
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc
	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc

	opt-9 -load ../../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-swap2.bc
#------------------------Code Generation------------	
	clang++-9 -O3 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-pthread.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/wifi-tx/bin/wifi-tx-pthread.so -n 1 -p 1



old:
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
	clang++-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test.native
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	clang++-9 -O1 -S -flto -emit-llvm ../../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged1.bc dummy.bc -o $(APP_NAME)-trans.bc	
	opt-9 -load ../../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap.bc
	clang++-9 -O3 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap.bc -o $(APP_NAME)-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/wifi-tx/bin/wifi-tx-serial.so -n 1 -p 1

omppass:
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

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc

	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

	clang++-9 -O1 -S -flto -emit-llvm ../../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	
	
	# opt-13 -load ../../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc

	opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc


	clang++-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-omp.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/wifi-tx/bin/wifi-tx-omp.so -n 1 -p 1
test_omp:
#-------------------create openMP parallel regions--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-staged2.bc -o $(APP_NAME)-test-omp.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test-omp.native

test_serial:
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test1.bc -o $(APP_NAME)-test-serial.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-test-serial.native


break_serial:
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	clang-9 -O1 -S -flto -emit-llvm $(CEDR_PATH)/libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-test1.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc
	clang-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-kernel-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	$(CEDR_PATH)/build/sub_dag -a $(CEDR_PATH)/applications/APIApps/wifi-tx/bin/$(APP_NAME)-kernel-serial.so -n 1 -p 1


break_omp:
	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

	clang++-9 -O1 -S -flto -emit-llvm $(CEDR_PATH)/libdash/dummy_function.cpp -o dummy.bc
	llvm-link $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc	

	opt-13 -enable-new-pm=0 -load $(OMP_PASS_SHARED) -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap2.bc


	clang++-13 -O3 -lm -lz -fopenmp=libiomp5 -lpthread -shared -fPIC $(APP_NAME)-swap2.bc -o $(APP_NAME)-omp.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	$(CEDR_PATH)/build/sub_dag -a $(CEDR_PATH)/applications/APIApps/wifi-tx/bin/wifi-tx-omp.so -n 1 -p 1

overhead:
	# $(TRACE_ATLAS_BUILD)/bin/KernelSerial -t raw.trc -k $(APP_NAME)-jr.json -b $(APP_NAME)-opt.bc -o $(APP_NAME)-dag.json
	$(TRACE_ATLAS_BUILD)/bin/PointBasedDAG -t raw.trc -k $(APP_NAME)-jr.json
clean:
	-rm $(APP_NAME)-*.so
	-rm $(APP_NAME)-*.out
	-rm $(APP_NAME)-*.bc
	-rm $(APP_NAME)-*.json
	-rm $(APP_NAME).tracer
	-rm $(APP_NAME)-*.native
	-rm LoopTraceFile.json
	-rm dummy.bc
