all:
	/heorot/lchang21/llvm-release/llvm-13/llvm/bin/clang-13 \
		-g \
		-shared \
		-fPIC \
		-Wall -fno-rtti \
		-I /heorot/lchang21/llvm-release/llvm-13/llvm/include/ \
		-I ../../../extern/include/\
		erase.cpp \
		omp-parallel.cpp \
		printBBName.cpp \
		api_swap.cpp \
		-o omp-parallel.so

printBB:
	clang-13 \
		-ggdb \
		-shared \
		-fPIC \
		-Wall -fno-rtti \
		-I /home/lchang21/llvm-13/llvm-13/include/ \
		-I /home/lchang21/llvm-13/llvm-13/include/llvm-c \
		-I ../../../extern/include/\
		printBBName.cpp \
		-o omp-parallel.so

llvm14:
	clang-14 \
		-ggdb \
		-shared \
		-fPIC \
		-Wall -fno-rtti \
		-I /home/lchang21/llvm-14/llvm-extrac/include/ \
		-I /home/lchang21/llvm-14/llvm-extrac/include/llvm-c \
		-I ../../../extern/include/\
		omp-parallel-staged.cpp \
		-o omp-parallel.so


clean:
	-rm omp-parallel.so
