# llvm 19 position
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin

# compile command

clang++ -fPIC -shared -o CodeExtractPass.so CodeExtractPass.cpp \
    `llvm-config --cxxflags --ldflags --system-libs --libs core passes analysis transformutils` -O3