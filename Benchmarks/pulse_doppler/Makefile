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

DAG_generation:
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
task_merging:
#-------------------DAG Transformation--------------
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -TaskMergingReorder -tm task_merging_schedule.json $(APP_NAME)-opt.bc -S -o $(APP_NAME)-merging.bc
	clang-9 -lm -lz -lpthread -I/mnt/nobackup-09/Dash/Sources/include/gsl/  -I ./ -lgsl -lgslcblas $(APP_NAME)-merging.bc -o $(APP_NAME)-merging.native -fuse-ld=lld $(TRACEATLAS_PASS_BACKEND_STATIC)
	./$(APP_NAME)-merging.native

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
