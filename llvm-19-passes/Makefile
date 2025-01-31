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
		`llvm-config --ldflags --system-libs --libs core passes analysis transformutils` \
		-o MergeTaskExtraction.so
		

clean:
	rm -f MergeTaskExtraction.so