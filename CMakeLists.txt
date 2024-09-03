cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0075 NEW)
project(code_chunk CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)

# Enable parallel compilation
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# Enable unity build
set(CMAKE_UNITY_BUILD ON)

# Enable Link Time Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if(supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO is not supported: ${error}")
endif()

# Include other CMake files
include(cmake/DetectArchitecture.cmake)
include(cmake/FindDependencies.cmake)
include(cmake/ConfigureLLAMACPP.cmake)
include(cmake/ConfigureCMakeLib.cmake)
include(cmake/ConfigureDoxygen.cmake)
include(cmake/ClangTidy.cmake)

# Find LLVM using llvm-config
find_program(LLVM_CONFIG llvm-config)
if(NOT LLVM_CONFIG)
  message(FATAL_ERROR "llvm-config not found!")
endif()

execute_process(COMMAND ${LLVM_CONFIG} --cxxflags
                OUTPUT_VARIABLE LLVM_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG} --ldflags
                OUTPUT_VARIABLE LLVM_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG} --libs all
                OUTPUT_VARIABLE LLVM_LIBS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG} --includedir
                OUTPUT_VARIABLE LLVM_INCLUDE_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG} --libdir
                OUTPUT_VARIABLE LLVM_LIB_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Add LLVM flags to CMAKE_CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

# Add LLVM include directory
include_directories(${LLVM_INCLUDE_DIR})

# Add LLVM library directory
link_directories(${LLVM_LIB_DIR})

# Optimize include directories
set(INCLUDE_DIRS
    ${LLVM_INCLUDE_DIR}
    ${CLANG_INCLUDE_DIRS}
    ${LLAMA_CPP_SOURCE_DIR}
    ${LLAMA_CPP_SOURCE_DIR}/common
    ${LLAMA_CPP_SOURCE_DIR}/include
    ${LLAMA_CPP_SOURCE_DIR}/examples
    ${LLAMA_CPP_SOURCE_DIR}/ggml/include
    ${LLAMA_CPP_SOURCE_DIR}/ggml/src
    ${LLAMA_CPP_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/cmake_headers
    ${CMAKE_LIB_SOURCE_DIR}/Source
    ${CMAKE_LIB_BINARY_DIR}/Source
    ${CMAKE_LIB_BINARY_DIR}/Utilities
    ${CMAKE_LIB_SOURCE_DIR}/Utilities
    ${CMAKE_LIB_SOURCE_DIR}/Utilities/std
    ${CMAKE_LIB_SOURCE_DIR}/Utilities/cm3p
    ${CMAKE_LIB_SOURCE_DIR}/Utilities/cm3p/values
    ${CMAKE_LIB_SOURCE_DIR}/Utilities/cmlibuv/include
    ${JSONCPP_INCLUDE_DIRS}
)
include_directories(BEFORE ${INCLUDE_DIRS})

# Add compile definitions
add_compile_definitions(
    CURL_STATICLIB
    LIBARCHIVE_STATIC
    CMAKE_USE_SYSTEM_CURL=1
    CMAKE_USE_SYSTEM_EXPAT=1
    CMAKE_USE_SYSTEM_ZLIB=1
    CMAKE_USE_SYSTEM_BZIP2=1
    CMAKE_USE_SYSTEM_LIBLZMA=1
    CMAKE_USE_SYSTEM_FORM=1
    CMAKE_USE_SYSTEM_JSONCPP=1
    CMAKE_USE_SYSTEM_LIBARCHIVE=1
    CMAKE_BOOTSTRAP=0
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

# Define the main executable
add_executable(code_chunk 
    src/main.cpp 
    src/ModelLoader.cpp 
    src/FunctionExtractor.cpp 
    src/FileUtil.cpp
    src/CMakeParser.cpp
    src/ASTUtils.cpp
    src/ClangUtils.cpp
    src/CodeAnalyzer.cpp
)

# Precompiled headers
set(PCH_HEADERS
    ${CMAKE_SOURCE_DIR}/src/pch.h
)
target_precompile_headers(code_chunk PRIVATE ${PCH_HEADERS})

add_dependencies(code_chunk llama_cpp cmake_lib copy_cmake_headers build_cmake_libs)

# Include JsonCpp headers
include_directories(${JSONCPP_INCLUDE_DIRS})

# Link libraries
include(cmake/LinkLibraries.cmake)

# Add compile options for JsonCpp
target_compile_options(code_chunk PRIVATE ${JSONCPP_CFLAGS})

# Link LLVM libraries
target_link_libraries(code_chunk PRIVATE ${LLVM_LDFLAGS} ${LLVM_LIBS})

# Add a custom command to run Doxygen after the build
add_custom_command(TARGET code_chunk POST_BUILD
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating API documentation with Doxygen"
                   VERBATIM)