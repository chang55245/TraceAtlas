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
$TRACEHOME/build/bin/dagExtractor -t raw.trc -k kernel-${1%.c}.json -o kernel-${1%.c}-dagExtractor.json

$TRACEHOME/build/bin/fatBinDump -i output-${1%.c}.ll -k kernel-${1%.c}.json -t kernel-${1%.c}-tik.json -d kernel-${1%.c}-dagExtractor.json -o output-${1%.c}-bindump.ll -o2 output-${1%.c}-application-arm.json
sed -i "s/output-${1%.c}.so/output-${1%.c}-arm.so/g" output-${1%.c}-application-arm.json
clang++-8 -march=armv8-a -target aarch64-linux-gnu -shared -fPIC -fuse-ld=lld-8 output-${1%.c}-bindump.ll -o output-${1%.c}-arm.so
