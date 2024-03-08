cmake_minimum_required(VERSION 3.10)
project(code_chunk)

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

# Configure Doxygen
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(CMAKE_PROJECT_DIR ${CMAKE_SOURCE_DIR})

# Request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
message("Doxygen build started")

# Optional: Add a custom target to run Doxygen when you build the project
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

# Enable Clang-Tidy checks
set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,cppcoreguidelines-*,bugprone-*,performance-*")

# # Optionally, include other checks or modify the checks as needed.
# # Adjust the header-filter to only include files from the src directory
set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -checks=${CLANG_TIDY_CHECKS};
    -header-filter=${CMAKE_SOURCE_DIR}/src/.*;
)

# Add the executable
# Update the source file paths to reflect their new location in the src directory
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

# Specify the full path to the common library
set(COMMON_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/common/libcommon.a")
set(LIBGGML_STATIC_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libggml_static.a")
set(LIBLLAMA_LIB_PATH "${CMAKE_SOURCE_DIR}/llama.cpp/build/libllama.a")

# Link against LLVM and Clang libraries
target_link_libraries(code_chunk
    PRIVATE
    clang-cpp
    LLVM
    LLVMSupport
    "/usr/lib/libclang.so.16"
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
