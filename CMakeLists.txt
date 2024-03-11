cmake_minimum_required(VERSION 3.10)
project(code_chunk)
include(ExternalProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")
set(LLVM_ENABLE_PROJECTS "clang")
# Detect the target architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(LLVM_TARGETS_TO_BUILD "X86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|AARCH64)$")
    set(LLVM_TARGETS_TO_BUILD "AArch64")
    # Add other architectures as needed
else()
    message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()
set(LLVM_BUILD_LLVM_DYLIB ON)
set(LLVM_LINK_LLVM_DYLIB ON)
set(LLVM_ENABLE_RTTI ON)
set(LLVM_ENABLE_EH ON)
set(LLVM_ENABLE_FFI ON)
set(BUILD_SHARED_LIBS ON)

# Find LLVM
find_package(LLVM 16 REQUIRED CONFIG)
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

find_library(LIBCLANG_LIBRARY
    NAMES libclang clang
    HINTS /opt/local/lib /usr/local/lib /usr/lib
)

# Configure Doxygen
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(CMAKE_PROJECT_DIR ${CMAKE_SOURCE_DIR})

ExternalProject_Add(llama_cpp
    GIT_REPOSITORY https://github.com/ggerganov/llama.cpp.git
    GIT_TAG master # Or specify a specific tag or branch if needed
    SOURCE_DIR "${CMAKE_BINARY_DIR}/llama_cpp"
    BINARY_DIR "${CMAKE_BINARY_DIR}/llama_cpp/build" # This directory will be created automatically
    CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/llama_cpp/build" && "${CMAKE_COMMAND}" -S "${CMAKE_BINARY_DIR}/llama_cpp" -B "${CMAKE_BINARY_DIR}/llama_cpp/build"
    BUILD_COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}/llama_cpp/build" -- -j24
    INSTALL_COMMAND "" # No install command, adjust if necessary
    TEST_COMMAND "" # No test command, adjust if necessary
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
    message("Doxygen need to be installed to generate the doxygen documentation")
endif()

# Enable Clang-Tidy checks
set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,cppcoreguidelines-*,bugprone-*,performance-*")
set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -checks=${CLANG_TIDY_CHECKS};
    -header-filter=${CMAKE_SOURCE_DIR}/src/.*;
)

# Add the executable
add_executable(code_chunk 
    src/main.cpp 
    src/ModelLoader.cpp 
    src/FunctionExtractor.cpp 
    src/FileUtil.cpp
)

target_include_directories(code_chunk PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(code_chunk SYSTEM PRIVATE
    ./llama.cpp
    ./llama.cpp/common/
)

add_dependencies(code_chunk llama_cpp)

# Specify the full path to the libraries
set(COMMON_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/common/libcommon.a")
set(LIBGGML_STATIC_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libggml_static.a")
set(LIBLLAMA_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libllama.a")

# Link against LLVM, Clang libraries, and the built libraries
target_link_libraries(code_chunk
    PRIVATE
    clang-cpp
    LLVM
    LLVMSupport
    ${LIBCLANG_LIBRARY}
    ${COMMON_LIB_PATH}
    ${LIBGGML_STATIC_LIB_PATH}
    ${LIBLLAMA_LIB_PATH}
    ${CMAKE_THREAD_LIBS_INIT}
)

add_custom_command(TARGET code_chunk POST_BUILD
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating API documentation with Doxygen"
                   VERBATIM)
