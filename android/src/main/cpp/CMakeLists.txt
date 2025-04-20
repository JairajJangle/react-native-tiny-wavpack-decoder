# android/src/main/cpp/CMakeLists.txt
cmake_minimum_required(VERSION 3.4.1)

# Add logging library for android logcat
find_library(log-lib log)

# Set the path to the tiny wavpack library source files
set(WAVPACK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tiny-wavpack/lib)

# Set the path to the common decoder interface implementation
set(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/tiny-wavpack/common)

# Add the tiny wavpack library source files
add_library(tiny_wavpack
            STATIC
            ${WAVPACK_SRC_DIR}/bits.c
            ${WAVPACK_SRC_DIR}/float.c
            ${WAVPACK_SRC_DIR}/metadata.c
            ${WAVPACK_SRC_DIR}/unpack.c
            ${WAVPACK_SRC_DIR}/words.c
            ${WAVPACK_SRC_DIR}/wputils.c)

# Include directories for the tiny wavpack library
target_include_directories(tiny_wavpack PUBLIC ${WAVPACK_SRC_DIR})

# Add the common decoder implementation
add_library(common_decoder
            STATIC
            ${COMMON_SRC_DIR}/TinyWavPackDecoderInterface.c)

# Includes both the common and WavPack headers
target_include_directories(common_decoder PUBLIC 
                          ${COMMON_SRC_DIR}
                          ${WAVPACK_SRC_DIR})

# Link wavpack library to common decoder
target_link_libraries(common_decoder tiny_wavpack)

# Add the JNI implementation
add_library(tiny-wavpack-decoder
            SHARED
            TinyWavPackDecoderModule.cpp)

# Include directories for the JNI implementation
target_include_directories(tiny-wavpack-decoder PUBLIC
                          ${COMMON_SRC_DIR}
                          ${WAVPACK_SRC_DIR})

# Link the libraries
target_link_libraries(tiny-wavpack-decoder
                     common_decoder
                     tiny_wavpack
                     ${log-lib}
                     android)