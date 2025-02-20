project(wifi-tx)

# Include the TraceAtlas functions
include(TraceAtlasFunctions)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/crc.c ${CMAKE_CURRENT_SOURCE_DIR}/src/txData.c ${CMAKE_CURRENT_SOURCE_DIR}/src/baseband_lib.c ${CMAKE_CURRENT_SOURCE_DIR}/src/viterbi.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/interleaver_deintleaver.c ${CMAKE_CURRENT_SOURCE_DIR}/src/scrambler_descrambler.c ${CMAKE_CURRENT_SOURCE_DIR}/src/qpsk_Mod_Demod.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/CyclicPrefix.c ${CMAKE_CURRENT_SOURCE_DIR}/src/Preamble_ST_LG.c ${CMAKE_CURRENT_SOURCE_DIR}/src/diag.c ${CMAKE_CURRENT_SOURCE_DIR}/src/rfInf.c ${CMAKE_CURRENT_SOURCE_DIR}/src/pilot.c 
	${CMAKE_CURRENT_SOURCE_DIR}/src/TX.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/fft_hs.cpp)

# Create input directory in build/pulse_doppler folder and copy input files
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/input/
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}/input)

# Use the functions for wifi-tx

set_compiler("clang++")
add_dag_generation_target(${PROJECT_NAME} "${SOURCES}")
add_task_merging_target(${PROJECT_NAME}) 