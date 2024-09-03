cmake_minimum_required(VERSION 3.10)

project(TestProject VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCE_FILES
    test.cpp
    # Add other source files here
)

# Add executable
add_executable(TestExecutable ${SOURCE_FILES})

# Link libraries if needed
# target_link_libraries(TestExecutable some_library)

# Install rules
install(TARGETS TestExecutable DESTINATION bin)