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

pipeline:
	/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang++ -std=c++17 -g `/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-config --cxxflags --ldflags --system-libs --libs core irreader passes` -o ExtrationPassPipeline ExtrationPassPipeline.cpp

clean:
	rm -f MergeTaskExtraction.so