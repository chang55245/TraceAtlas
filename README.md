# build

git submodule update --init --recursive

sh bootstrap-vcpkg.sh


cmake ../ -DLLVM_DIR=/heorot/lchang21/llvm-release/llvm-9/llvm-9/lib/cmake/llvm -DCMAKE_TOOLCHAIN_FILE=/heorot/lchang21/TraceAtlas/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug

