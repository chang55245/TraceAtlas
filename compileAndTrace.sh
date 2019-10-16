#!/bin/sh

if [ "$#" -ne 1 ]; then
  echo "Requires C file to process"
  exit 1
fi

TRACEHOME=/localhome/jmack2545/rcl/DASH-SoC/TraceAtlas

clang-8 -S -flto -static -fuse-ld=lld-8 $1 -o output-${1%.c}.ll
opt-8 -load $TRACEHOME/build/lib/DashPasses.so -EncodedTrace output-${1%.c}.ll -S -o output-${1%.c}-opt.ll
clang++-8 -static -fuse-ld=lld-8 output-${1%.c}-opt.ll $TRACEHOME/build/lib/libDashTracer.a -lpthread -lz -lc -o ${1%.c}.out

./${1%.c}.out

python $TRACEHOME/Python/KernelDetector/main.py -i raw.trc -j kernel-${1%.c}.json

$TRACEHOME/build/bin/tik -p -j kernel-${1%.c}.json -o kernel-${1%.c}-tik.json -t LLVM ./output-${1%.c}.ll
