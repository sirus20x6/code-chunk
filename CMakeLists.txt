cmake_minimum_required(VERSION 3.10)
project(FunctionParser)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

# Enable Clang-Tidy checks
set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,cppcoreguidelines-*,bugprone-*,performance-*")

# Optionally, include other checks or modify the checks as needed.
set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -checks=${CLANG_TIDY_CHECKS};
    -header-filter=.;
)

# Add the executable
add_executable(FunctionParser main.cpp ModelLoader.cpp FunctionExtractor.cpp FileUtil.cpp)

# Include directories for header files
target_include_directories(FunctionParser PRIVATE
    ./llama.cpp
    ./llama.cpp/common/
)

# Specify the full path to the common library
set(COMMON_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/common/libcommon.a")
set(LIBGGML_STATIC_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libggml_static.a")
set(LIBLLAMA_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libllama.a")

# Link against LLVM and Clang libraries
target_link_libraries(FunctionParser
    PRIVATE
    clang-cpp
    LLVM
    LLVMSupport
    "/usr/lib/libclang.so.16"
    ${COMMON_LIB_PATH}
    ${LIBGGML_STATIC_LIB_PATH}
    ${LIBLLAMA_LIB_PATH}
    llama
    ${CMAKE_THREAD_LIBS_INIT}
)

# Note: The -header-filter=.; option tells clang-tidy to only apply checks to the headers in the current directory.
# Adjust this according to your project's layout and requirements.
