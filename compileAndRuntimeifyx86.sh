#!/bin/sh

if [ "$#" -ne 1 ]; then
  echo "Requires C file to process"
  exit 1
fi

TRACEHOME=/localhome/jmack2545/rcl/DASH-SoC/TraceAtlas

# Build the instrumented binary
clang-9 -S -flto -static -fuse-ld=lld-9 $1 -o output-${1%.c}.ll
opt-9 -load $TRACEHOME/build/lib/AtlasPasses.so -EncodedTrace output-${1%.c}.ll -S -o output-${1%.c}-opt.ll
clang++-9 -static -fuse-ld=lld-9 output-${1%.c}-opt.ll $TRACEHOME/build/lib/libAtlasBackend.a -lpthread -lz -lc -o ${1%.c}.out

# Collect trace output
./${1%.c}.out

# Perform trace analysis and tik extraction
python $TRACEHOME/Python/KernelDetector/main.py -i raw.trc -j kernel-${1%.c}.json

$TRACEHOME/build/bin/tik -p -j kernel-${1%.c}.json -S -f LLVM -o kernel-${1%.c}-tik.ll -t LLVM ./output-${1%.c}.ll
$TRACEHOME/build/bin/dagExtractor -t raw.trc -k kernel-${1%.c}.json -o kernel-${1%.c}-dagExtractor.json

# Perform inlining of memory functions
opt-9 -inline -S -o ./kernel-${1%.c}-tik-inline.ll ./kernel-${1%.c}-tik.ll

# Output final results for the runtime
#$TRACEHOME/build/bin/fatBinDump -i output-${1%.c}.ll -k kernel-${1%.c}.json -t kernel-${1%.c}-tik.json -d kernel-${1%.c}-dagExtractor.json -o output-${1%.c}-bindump.ll -o2 output-${1%.c}-application-x86.json
#sed -i "s/output-${1%.c}.so/output-${1%.c}-x86.so/g" output-${1%.c}-application-x86.json
#clang++-9 -shared -fPIC -fuse-ld=lld-9 output-${1%.c}-bindump.ll -o output-${1%.c}-x86.so
#clang++-9 -shared -fPIC -fuse-ld=lld-9 ./kernel-${1%.c}-tik-inline.ll -o output-${1%.c}-x86.so
