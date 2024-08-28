cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0075 NEW)
project(code_chunk)
include(ExternalProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")

# Detect the target architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(LLVM_TARGETS_TO_BUILD "X86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|AARCH64)$")
    set(LLVM_TARGETS_TO_BUILD "AArch64")
else()
    message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Find LLVM
find_package(LLVM 17 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find Clang
find_package(Clang REQUIRED CONFIG)
message(STATUS "Found Clang")

# Include LLVM and Clang headers
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Add LLVM and Clang definitions
add_definitions(${LLVM_DEFINITIONS})
add_compile_definitions(GGML_USE_HIPBLAS)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(
    -Wno-pointer-arithmetic
    -Wno-sign-conversion
    -Wno-narrowing
    -Wno-implicit-int-float-conversion
    -Wno-implicit-float-conversion
    -Wno-bugprone-easily-swappable-parameters
  )
endif()

# Configure Doxygen
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(CMAKE_PROJECT_DIR ${CMAKE_SOURCE_DIR})

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

# Remove any system-wide include directories
include_directories(BEFORE
    ${CMAKE_SOURCE_DIR}/src
    ${LLAMA_CPP_SOURCE_DIR}
    ${LLAMA_CPP_SOURCE_DIR}/examples
    ${LLAMA_CPP_SOURCE_DIR}/ggml/include
    ${LLAMA_CPP_SOURCE_DIR}/ggml/src
    ${LLAMA_CPP_SOURCE_DIR}/src
)

# Request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
message("Doxygen build started")

# Optional: Add a custom target to run Doxygen when you build the project
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else()
    message("Doxygen needs to be installed to generate the doxygen documentation")
endif()

# Enable Clang-Tidy checks
set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,performance-*")
#set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,cppcoreguidelines-*,bugprone-*,performance-*")
set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -checks=${CLANG_TIDY_CHECKS};
    -header-filter=${CMAKE_SOURCE_DIR}/src/.*;
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

add_executable(code_chunk 
    src/main.cpp 
    src/ModelLoader.cpp 
    src/FunctionExtractor.cpp 
    src/FileUtil.cpp
)

add_dependencies(code_chunk llama_cpp)

# Explicitly set include directories for the target
target_include_directories(code_chunk PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${LLAMA_CPP_SOURCE_DIR}
    ${LLAMA_CPP_SOURCE_DIR}/common
    ${LLAMA_CPP_SOURCE_DIR}/include
    ${LLAMA_CPP_SOURCE_DIR}/ggml/include
    ${LLAMA_CPP_SOURCE_DIR}/examples
    ${LLAMA_CPP_SOURCE_DIR}/ggml/src
    ${LLAMA_CPP_SOURCE_DIR}/src
)

# Specify the full path to the libraries
set(COMMON_STATIC_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/common/libcommon.a")
set(LIBGGML_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/ggml/src/libggml.so")
set(LIBLLAMA_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/src/libllama.so")
#set(LIBLLAVA_LIB_PATH "${LLAMA_CPP_BINARY_DIR}/examples/libllava_shared.so")


add_definitions(-DLLAMA_API=)
add_definitions(-DGGML_API=)
# Link against the built libraries first, then LLVM and Clang libraries
target_link_libraries(code_chunk
    PRIVATE
    ${COMMON_STATIC_LIB_PATH}
    ${LIBGGML_LIB_PATH}
    ${LIBLLAMA_LIB_PATH}
    clang-cpp
    LLVM
    LLVMSupport
    ${LIBCLANG_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    libclang
)


# Add a custom command to run Doxygen after the build
add_custom_command(TARGET code_chunk POST_BUILD
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating API documentation with Doxygen"
                   VERBATIM)