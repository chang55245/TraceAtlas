all:
	clang++-9 \
		-ggdb \
		-shared \
		-fPIC \
		-fno-rtti \
		-I /heorot/lchang21/llvm-release/llvm-9/llvm-9/include \
		-I ../../../extern/include/\
		api_swap.cpp \
		function_inlining.cpp \
		-o api_swap_pass.so

clean:
	-rm api_swap_pass.so
