set(SOURCES BackendTrace.c LoopTrace.cpp BackendKernelLocation.cpp BackendMemProfile.cpp)
add_library(AtlasBackend STATIC ${SOURCES})
set_target_properties(
    AtlasBackend PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    OUTPUT_NAME AtlasBackend
)

target_link_libraries(AtlasBackend ${llvm_libs} AtlasUtil ZLIB::ZLIB nlohmann_json nlohmann_json::nlohmann_json)
target_include_directories(AtlasBackend PUBLIC ${TRACE_INC})
if(WIN32)
    target_compile_options(AtlasBackend PRIVATE -fno-rtti -W3 -Wextra -Wconversion -fPIC)
else()
    target_compile_options(AtlasBackend PRIVATE -fno-rtti -Wall -Wextra -Wconversion -fPIC)
endif()

install(TARGETS AtlasBackend
    ARCHIVE DESTINATION lib
)

file(GLOB GR ${TRACE_INC}/Backend/*.h)
install (FILES ${GR} DESTINATION "include/Backend")