include(ExternalProject)

# Configure the llama.cpp project as an external project
set(LLAMA_CPP_SOURCE_DIR "${CMAKE_BINARY_DIR}/llama_cpp-src")
set(LLAMA_CPP_BINARY_DIR "${CMAKE_BINARY_DIR}/llama_cpp-build")

ExternalProject_Add(llama_cpp
    GIT_REPOSITORY https://github.com/ggerganov/llama.cpp.git
    GIT_TAG master
    SOURCE_DIR "${LLAMA_CPP_SOURCE_DIR}"
    BINARY_DIR "${LLAMA_CPP_BINARY_DIR}"
    CMAKE_ARGS
        -DCMAKE_C_COMPILER=/opt/rocm/llvm/bin/clang
        -DCMAKE_CXX_COMPILER=/opt/rocm/llvm/bin/clang++
        -DLLAMA_HIPBLAS=ON
        -DAMDGPU_TARGETS=gfx1100
        -DCMAKE_BUILD_TYPE=Release
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release -j 50
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

# Specify the full path to the libraries
set(COMMON_STATIC_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/common/libcommon.a")
set(LIBGGML_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/ggml/src/libggml.so")
set(LIBLLAMA_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/src/libllama.so")

add_definitions(-DLLAMA_API=)
add_definitions(-DGGML_API=)