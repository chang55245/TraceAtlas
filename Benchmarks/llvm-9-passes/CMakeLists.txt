# Add LLVM includes
include_directories(
    ${LLVM_9_PATH}/include
    ${CEDR_PATH}/extern/include
)

# Create function inlining pass
add_library(llvm9_passes SHARED
    function_inlining.cpp
    api_swap.cpp
)

set_target_properties(llvm9_passes PROPERTIES
    COMPILE_FLAGS "-g -shared -fPIC -fno-rtti"
    PREFIX ""
)