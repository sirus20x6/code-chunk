# Enable Clang-Tidy checks
# Uncomment the following lines to enable Clang-Tidy

#set(CLANG_TIDY_CHECKS "-*,clang-analyzer-*,performance-*")
#set(CMAKE_CXX_CLANG_TIDY 
#    clang-tidy;
#    -checks=${CLANG_TIDY_CHECKS};
#    -header-filter=${CMAKE_SOURCE_DIR}/src/.*;
#)