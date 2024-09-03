
# Find required packages
find_package(LibLZMA REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBIDN2 REQUIRED libidn2)

target_link_libraries(code_chunk
    PRIVATE
    ${COMMON_STATIC_LIB_PATH}
    ${LIBGGML_LIB_PATH}
    ${LIBLLAMA_LIB_PATH}
    LLVM
    LLVMSupport
    ${LIBCLANG_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBCMSTD}
    ${LIBCMLIBRHASH}
    ${LIBCMZLIB}
    ${LIBCMCURL}
    ${LIBCMNGHTTP2}
    ${LIBCMEXPAT}
    ${LIBCMBZIP2}
    ${LIBCMZSTD}
    ${LIBCMLIBLZMA}
    ${LIBCMLIBARCHIVE}
    ${LIBCMJSONCPP}
    ${LIBCMLIBUV}
    ${LIBCMCPPDAP}
    ${LIBCMLLPKGC}
    ${LIBCMSYS_C}
    ${LIBCMSYS}
    ${LIBCMFORM}
    ${LIBCMAKELIB}
    ${LIBCPACKLIB}
    ${LIBCTESTLIB}
    libclang
    ${LIBLZMA_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LIBIDN2_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${LIBIDN2_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${llvm_libs}
    -lz
    -lbz2
)


# Add include directories
target_include_directories(code_chunk PRIVATE 
    ${LIBLZMA_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${LIBIDN2_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_LIB_SOURCE_DIR}/Utilities/cmzlib
)

# Add compile definitions
target_compile_definitions(code_chunk PRIVATE 
    ${LIBLZMA_DEFINITIONS}
    ${ZLIB_DEFINITIONS}
    ${LIBIDN2_DEFINITIONS}
    ${OPENSSL_DEFINITIONS}
)

# If using CMake's custom zlib, add it explicitly
if(EXISTS "${CMAKE_LIB_BINARY_DIR}/Utilities/cmzlib/libcmzlib.a")
    target_link_libraries(code_chunk PRIVATE ${CMAKE_LIB_BINARY_DIR}/Utilities/cmzlib/libcmzlib.a)
endif()