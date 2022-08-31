cmake_minimum_required(VERSION 3.19)

project(end-to-end-example CXX)

# Create executable
add_executable(get_random get_random.cpp)

# Set LIBRARY_PATH macro for get_random.cpp
target_compile_definitions(get_random PRIVATE LIBRARY_PATH="${PROJECT_SOURCE_DIR}/../package/lib/libqryptoki.so")

# For "cryptoki.h"
target_include_directories(get_random PRIVATE "../inc")

# For dlopen, etc.
target_link_libraries(get_random PRIVATE ${CMAKE_DL_LIBS})
