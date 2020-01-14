#!/bin/sh

if [ "$#" -ne 1 ]; then
  echo "Requires C file to process"
  exit 1
fi

TRACEHOME=/localhome/jmack2545/rcl/DASH-SoC/TraceAtlas

# Build the instrumented binary
clang-9 -S -flto -fPIC -ggdb -static -fuse-ld=lld-9 $1 -o output-${1%.c}.ll
opt-9 -load $TRACEHOME/build/lib/AtlasPasses.so -EncodedAnnotate output-${1%.c}.ll -S -o output-${1%.c}-annotate.ll
opt-9 -load $TRACEHOME/build/lib/AtlasPasses.so -EncodedTrace output-${1%.c}.ll -S -o output-${1%.c}-opt.ll
clang++-9 -static -fuse-ld=lld-9 output-${1%.c}-opt.ll $TRACEHOME/build/lib/libAtlasBackend.a -lpthread -lz -lc -o ${1%.c}.out

# Collect trace output
./${1%.c}.out

# Perform trace analysis and tik extraction
$TRACEHOME/build/bin/cartographer -i raw.trc -k kernel-${1%.c}.json
$TRACEHOME/build/bin/dagExtractor -t raw.trc -k kernel-${1%.c}.json -o kernel-${1%.c}-dagExtractor.json

$TRACEHOME/build/bin/fatBinDump -a output-${1%.c}-annotate.ll -k kernel-${1%.c}.json -d kernel-${1%.c}-dagExtractor.json -n ${1%.c} -o output-${1%.c}-extracted.ll -o2 ${1%.c}.json
clang-9 -shared -fPIC -ggdb output-${1%.c}-extracted.ll -o ${1%.c}.so

#$TRACEHOME/build/bin/tik -p -j kernel-${1%.c}.json -S -f LLVM -o kernel-${1%.c}-tik.ll -t LLVM ./output-${1%.c}.ll
#opt-9 -inline -S -o ./kernel-${1%.c}-tik-inline.ll ./kernel-${1%.c}-tik.ll
