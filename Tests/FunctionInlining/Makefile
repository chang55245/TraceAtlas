
TRACE_ATLAS_BUILD ?= /home/lchang21/cedr-traceAtlas/TraceAtlas/build
TRACEATLAS_PASS_SHARED = $(TRACE_ATLAS_BUILD)/lib/AtlasPasses.so

test: 
	clang++-9 -fno-discard-value-names test.cpp -emit-llvm -S -o test.ll
	opt-9 -load $(TRACEATLAS_PASS_SHARED) -DynmKernelLocating test.ll -S -o test-1.ll
clean:  
	rm test*.ll