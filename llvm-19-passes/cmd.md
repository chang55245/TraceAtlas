# llvm 19 position
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin

# compile command

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/clang-19 \
        -g \
        -shared \
        -fPIC \
        -std=c++17 \
        -Wall -fno-rtti \
        -I /heorot/lchang21/llvm-release/llvm-19/llvm-19/include/ \
        -I ../vcpkg/installed/x64-linux/include \
        MergeTaskExtraction.cpp \
        -o MergeTaskExtraction.so

# test command

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/opt \
    -load-pass-plugin=/heorot/lchang21/TraceAtlas/llvm-19-passes/TaskExtraction.so \
    -passes="MergeTaskExtraction" \
    -tm task_merging_schedule.json \
    pulse_doppler-merging.bc -S -o pulse_doppler-extraction.bc


# flags why no needed?
`llvm-config --ldflags --system-libs --libs core passes analysis transformutils` \
maybe these are already included in the clang-19 command?


# function extraction llvm-extract

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-extract \
    -S \
    pulse_doppler-extraction.bc \
    -func=main \
    -o main-extracted.bc

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-extract \
    -S \
    --delete \
    pulse_doppler-extraction.bc \
    -func=main \
    -o main-extracted-no-main.bc

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/opt \
    -load-pass-plugin=/heorot/lchang21/TraceAtlas/llvm-19-passes/TaskExtraction.so \
    -passes="remove-main" \
    pulse_doppler-extraction.bc -S -o pulse_doppler-no-main.bc

# function extraction llvm-dis

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-dis \
    main-extracted.bc \
    -o main-extracted.ll

# convert the extracted function to mlir

/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/mlir-translate \
    -import-llvm main-extracted.ll \
    -mlir-print-debuginfo \
    -o main-extracted.mlir

# test link
/heorot/lchang21/llvm-release/llvm-19/llvm-19/bin/llvm-link -S /heorot/lchang21/TraceAtlas/llvm-19-passes/main-extracted.ll /heorot/lchang21/TraceAtlas/llvm-19-passes/main-extracted-no-main.bc -o pulse_doppler_link.bc