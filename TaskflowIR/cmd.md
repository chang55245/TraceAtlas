cmake -G "Unix Makefiles" \
    -DMLIR_DIR=/heorot/lchang21/llvm-release/llvm-19/llvm-19/lib/cmake/mlir \
    -Dzstd_DIR=/heorot/lchang21/zstd/install/lib/cmake/zstd \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ..