# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find Clang
find_package(Clang REQUIRED CONFIG)
message(STATUS "Found Clang")
# Find required packages
find_package(Threads REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(LibLZMA REQUIRED)
find_package(ZLIB REQUIRED)

# Include LLVM and Clang headers
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Add LLVM and Clang definitions
add_definitions(${LLVM_DEFINITIONS})
add_compile_definitions(GGML_USE_HIPBLAS)

# Find JsonCpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)