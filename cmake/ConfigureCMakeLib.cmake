include(ExternalProject)

# Find JsonCpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Configure the CMake library as an external project
set(CMAKE_LIB_SOURCE_DIR "${CMAKE_BINARY_DIR}/cmake-src")
set(CMAKE_LIB_BINARY_DIR "${CMAKE_BINARY_DIR}/cmake-build")

# Define the paths for the CMake libraries
set(LIBCMSTD "${CMAKE_LIB_BINARY_DIR}/Utilities/std/libcmstd.a")
set(LIBCMLIBRHASH "${CMAKE_LIB_BINARY_DIR}/Utilities/cmlibrhash/libcmlibrhash.a")
set(LIBCMZLIB "${CMAKE_LIB_BINARY_DIR}/Utilities/cmzlib/libcmzlib.a")
set(LIBCMCURL "${CMAKE_LIB_BINARY_DIR}/Utilities/cmcurl/lib/libcmcurl.a")
set(LIBCMNGHTTP2 "${CMAKE_LIB_BINARY_DIR}/Utilities/cmnghttp2/libcmnghttp2.a")
set(LIBCMEXPAT "${CMAKE_LIB_BINARY_DIR}/Utilities/cmexpat/libcmexpat.a")
set(LIBCMBZIP2 "${CMAKE_LIB_BINARY_DIR}/Utilities/cmbzip2/libcmbzip2.a")
set(LIBCMZSTD "${CMAKE_LIB_BINARY_DIR}/Utilities/cmzstd/libcmzstd.a")
set(LIBCMLIBLZMA "${CMAKE_LIB_BINARY_DIR}/Utilities/cmliblzma/libcmliblzma.a")
set(LIBCMLIBARCHIVE "${CMAKE_LIB_BINARY_DIR}/Utilities/cmlibarchive/libarchive/libcmlibarchive.a")
set(LIBCMJSONCPP "${CMAKE_LIB_BINARY_DIR}/Utilities/cmjsoncpp/libcmjsoncpp.a")
set(LIBCMLIBUV "${CMAKE_LIB_BINARY_DIR}/Utilities/cmlibuv/libcmlibuv.a")
set(LIBCMCPPDAP "${CMAKE_LIB_BINARY_DIR}/Utilities/cmcppdap/libcmcppdap.a")
set(LIBCMLLPKGC "${CMAKE_LIB_BINARY_DIR}/Utilities/cmllpkgc/libcmllpkgc.a")
set(LIBCMSYS_C "${CMAKE_LIB_BINARY_DIR}/Source/kwsys/libcmsys_c.a")
set(LIBCMSYS "${CMAKE_LIB_BINARY_DIR}/Source/kwsys/libcmsys.a")
set(LIBCMFORM "${CMAKE_LIB_BINARY_DIR}/Source/CursesDialog/form/libcmForm.a")
set(LIBCMAKELIB "${CMAKE_LIB_BINARY_DIR}/Source/libCMakeLib.a")
set(LIBCPACKLIB "${CMAKE_LIB_BINARY_DIR}/Source/libCPackLib.a")
set(LIBCTESTLIB "${CMAKE_LIB_BINARY_DIR}/Source/libCTestLib.a")

ExternalProject_Add(cmake_lib
    GIT_REPOSITORY https://github.com/Kitware/CMake.git
    GIT_TAG master
    SOURCE_DIR "${CMAKE_LIB_SOURCE_DIR}"
    BINARY_DIR "${CMAKE_LIB_BINARY_DIR}"
    CMAKE_ARGS
        -DCMAKE_C_COMPILER=/opt/rocm/llvm/bin/clang
        -DCMAKE_CXX_COMPILER=/opt/rocm/llvm/bin/clang++
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_TESTING=OFF
        -DCMAKE_USE_SYSTEM_LIBRARIES=ON
        -DCMAKE_USE_SYSTEM_ZLIB=ON
        -DCMAKE_USE_SYSTEM_BZIP2=ON
        -DCMAKE_USE_SYSTEM_CURL=ON
        -DCMAKE_USE_SYSTEM_EXPAT=ON
        -DCMAKE_USE_SYSTEM_LIBARCHIVE=ON
        -DCMAKE_PREFIX_PATH=${JSONCPP_PREFIX}
        -DCMAKE_INCLUDE_PATH=${JSONCPP_INCLUDE_DIRS}
        -DCMAKE_LIBRARY_PATH=${JSONCPP_LIBRARY_DIRS}
        -DCMAKE_CXX_STANDARD=17
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_BOOTSTRAP=ON
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release -j 50
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)

# Add custom command to copy CMake headers
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/cmake_headers
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_LIB_SOURCE_DIR}/Source
            ${CMAKE_BINARY_DIR}/cmake_headers
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_LIB_SOURCE_DIR}/Utilities
            ${CMAKE_BINARY_DIR}/cmake_headers/Utilities
    DEPENDS cmake_lib
)

add_custom_target(copy_cmake_headers DEPENDS ${CMAKE_BINARY_DIR}/cmake_headers)

# Add custom target to ensure CMake libraries are built
add_custom_target(build_cmake_libs
    DEPENDS cmake_lib
    COMMAND ${CMAKE_COMMAND} -E echo "CMake libraries built successfully"
)