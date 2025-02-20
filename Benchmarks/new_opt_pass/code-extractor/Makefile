all:
	clang++-9 \
		-ggdb \
		-shared \
		-fPIC \
		-I /usr/include/llvm-9 \
		-I /usr/include/llvm-c-9 \
		-I ../../../extern/include/\
		code-extractor-test.cpp \
		-o code-extractor-test.so

test:
	opt-9 \
	-load \
	./code-extractor-test.so \
	-ExtractorTest test.bc -S -o test-res.bc	
clean:
	-rm test
