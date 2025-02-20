# Add LLVM includes
include_directories(
    ${LLVM_13_PATH}/include
    ${CEDR_PATH}/extern/include
)

# Create function inlining pass
add_library(llvm13_passes SHARED
    erase.cpp
    omp-parallel.cpp
    printBBName.cpp
    api_swap.cpp
)

set_target_properties(llvm13_passes PROPERTIES
    COMPILE_FLAGS "-g -shared -fPIC -fno-rtti"
    PREFIX ""
)