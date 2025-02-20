#ifeq ($(ARCH), aarch64)
#	CXX = aarch64-linux-gnu-g++
#	CC = aarch64-linux-gnu-gcc
#	RUNTIME_LIBDIR = ../../../build-arm/libdash
#endif

DEFINES = -DPRINT_IMAGES

# Tool specific make commands

ARCH ?= x86

LIBDASH_INC ?= ../../../libdash
LIBDASH_LIB ?= ../../../libdash/build_clang
RUNTIME_LIBDIR = ../../../build/libdash
STANDALONE_LIBDIR = $(RUNTIME_LIBDIR)

LIBDASH_POS ?= ../../../libdash/build_clang/libdash.a
LIBDASH_BC ?= ../../../libdash/build_clang/dash.a

INCLUDES = -I $(LIBDASH_INC)
LIBS = -L $(LIBDASH_LIB)

APP_NAME = lane_detection
SOURCES = conv.cpp track.cpp

$(APP_NAME)-$(ARCH).so:
	$(CC) $(CFLAGS) $(INCLUDES) -shared -fPIC $(SOURCES) -o $(APP_NAME)-$(ARCH).so

TRACE_ATLAS_BUILD = /localhome/suluhan/parallel-tool/traceAtlas/TraceAtlas/build
CEDR_PATH = /localhome/suluhan/parallel-tool/CEDR/zynq_hardware_emulator
DASH_ARCHIVES_LIB ?= ../../../dash-archives
GSL_LLVM_LIB ?= -L $(DASH_ARCHIVES_LIB)/release/gsl/lib
GSL_ARCHIVE ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/libgsl.a
GSL_OBJECT_DIR ?= $(DASH_ARCHIVES_LIB)/release/gsl/lib/temp

FUNTION_INLINING_PASS_SHARED = $(CEDR_PATH)/applications/experiments_and_tests/new_opt_pass/api_swap_pass.so
TRACEATLAS_PASS_SHARED = $(TRACE_ATLAS_BUILD)/lib/AtlasPasses.so
TRACEATLAS_PASS_BACKEND_STATIC = $(TRACE_ATLAS_BUILD)/lib/libAtlasBackend.a
CEDR_INTERFACE = ../../../libdash/dash.cpp ../../../libdash/kestrel.cpp

standalone: conv.cpp track.cpp
	$(CC) $(INCLUDES) -L $(STANDALONE_LIBDIR) $(DEFINES) -DSERIAL $(SOURCES) -l:libdash.a -lstdc++ -lm -o standalone.out -g

serial:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE)  $(SOURCES) -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem  $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

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
# clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	clang-9 -O1 -S -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged1.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	
	opt-9 -load ../../experiments_and_tests/new_opt_pass/api_swap_pass.so -SwapAPIs $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap.bc
#------------------------Code Generation------------	
	clang-9 -O3 -lm -lz -lpthread -shared -fPIC $(APP_NAME)-swap.bc -o $(APP_NAME)-serial.so $(TRACEATLAS_PASS_BACKEND_STATIC)
	../../../build/sub_dag -a $(CEDR_PATH)/applications/APIApps/lane_detection/$(APP_NAME)-serial.so -n 10 -p 1

test_serial:
	opt-13 -enable-new-pm=0 -load $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-test.bc -S -o $(APP_NAME)-test1.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test1.bc -o $(APP_NAME)-serial.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-serial.native

new:
#-------------------compile the application--------------
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) $(SOURCES) -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

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
# clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test.bc -o $(APP_NAME)-test.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
# ./$(APP_NAME)-test.native
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelStage -dg $(APP_NAME)-dag.json $(APP_NAME)-test.bc -S -o $(APP_NAME)-staged1.bc
	
test_new:	
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -NonKernelOutliner -jr $(APP_NAME)-jr.json -dg $(APP_NAME)-dag.json $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc	
	clang++-9 -g -O1 -S  -flto -emit-llvm ../../../libdash/dummy_function.cpp -o dummy.bc
	llvm-link-9 $(APP_NAME)-staged2.bc dummy.bc -S -o $(APP_NAME)-trans.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -SwapNonkernel -dg $(APP_NAME)-dag.json $(APP_NAME)-trans.bc -S -o $(APP_NAME)-swap1.bc
	opt-13 -enable-new-pm=0 -load $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-swap1.bc -S -o $(APP_NAME)-test0.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-pthread.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-pthread.native

omppass:
	clang-9 -Xclang -disable-O0-optnone -fPIC -DCPU_ONLY -flto -lgsl -lgslcblas -fuse-ld=lld-9 -Wl,-plugin-opt=emit-llvm $(INCLUDES)  $(CEDR_INTERFACE) $(SOURCES) -o $(APP_NAME)-link.bc
#-------------------function inlining and loop unrolling--------------
	opt-9 -load $(FUNTION_INLINING_PASS_SHARED) -DynmFunctionInlining $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -KernelLocating $(APP_NAME)-link.bc -S -o $(APP_NAME)-link.bc
	clang++-9 $(INCLUDES) -fuse-ld=lld-9 $(APP_NAME)-link.bc -lm -lz -lpthread -lgsl -lgslcblas $(TRACEATLAS_PASS_BACKEND_STATIC) -o $(APP_NAME).tracer
	./$(APP_NAME).tracer

	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmLoopUnroll -lt LoopTraceFile.json $(APP_NAME)-link.bc -S -o $(APP_NAME)-unroll.bc
#-------------------Tracing--------------
	opt-9 -mem2reg -sccp $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc
	opt-9 -simplifycfg -adce -reg2mem $(APP_NAME)-unroll.bc -S -o $(APP_NAME)-unroll.bc

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

test_omp:
	opt-13 -enable-new-pm=0 -load $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -OmpParallel $(APP_NAME)-staged1.bc -S -o $(APP_NAME)-staged2.bc

# test0:
# #-------------------run the binary for testing--------------
# 	# opt-13 -enable-new-pm=0 -load /home/lchang21/cedr-josh/zynq_hardware_emulator/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -printBB -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-staged2.bc

	opt-13 -enable-new-pm=0 -load $(CEDR_PATH)/applications/experiments_and_tests/llvm-13-omp-pass/omp-parallel.so -dg $(APP_NAME)-dag.json -jr $(APP_NAME)-jr.json  -erase $(APP_NAME)-staged2.bc -S -o $(APP_NAME)-test0.bc

	clang++-13 -fopenmp=libiomp5 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-test0.bc -o $(APP_NAME)-omp.native -fuse-ld=lld-9 $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-omp.native

clean:
	-rm *.so
	-rm *.out
	-rm *_img*.png
	-rm o*.jpeg
	-rm o*.png
	-rm lanes.png
	-rm $(APP_NAME)-*.bc
	-rm $(APP_NAME)-*.json
	-rm $(APP_NAME)-*.tracer
	-rm $(APP_NAME)-*.native
	-rm $(APP_NAME)-*.o
	-rm $(APP_NAME)*.tracer
	-rm LoopTraceFile.json
	-rm dummy.bc
	-rm raw.trc

