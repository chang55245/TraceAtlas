#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Clean build and rebuild
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Add environment variables for better debugging
export LLVM_DEBUG=1
# Modify ASAN options to detect more issues and generate better stack traces



./LoopUnrollTest
echo "Done" 