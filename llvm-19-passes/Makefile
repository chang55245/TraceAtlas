pass:
	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang-19 \
		-g \
		-shared \
		-fPIC \
		-std=c++17 \
		-Wall -fno-rtti \
		-I /heorot/lchang21/llvm-release/llvm-19/llvm-19/include/ \
		-I ../vcpkg/installed/x64-linux/include \
		MergeTaskExtraction.cpp \
		-o MergeTaskExtraction.so
test:
	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/opt \
    -load-pass-plugin=/heorot/lchang21/TraceAtlas/llvm-19-passes/MergeTaskExtraction.so \
    -passes="MergeTaskExtraction" \
    -tm task_merging_schedule.json \
    pulse_doppler-merging.bc -S -o pulse_doppler-extraction.bc

	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-extract \
    pulse_doppler-extraction.bc \
    -func=main \
    -o main-extracted.bc

	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-dis \
    main-extracted.bc \
    -o main-extracted.ll

	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/mlir-translate \
		-import-llvm main-extracted.ll \
		-mlir-print-debuginfo \
		-o main-extracted.mlir
clean:
	rm -f MergeTaskExtraction.so