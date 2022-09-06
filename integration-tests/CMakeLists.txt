cmake_minimum_required(VERSION 3.19)

project(integration-tests CXX)

# Create executable
add_executable(run_tests
    test-sources/common.cpp
    test-sources/InitializeTests.cpp
    test-sources/FinalizeTests.cpp
    test-sources/GetInfoTests.cpp
    test-sources/GetFunctionListTests.cpp
    test-sources/SeedRandomTests.cpp
    test-sources/GenerateRandomTests.cpp
    run_tests.cpp
)

target_include_directories(run_tests PRIVATE
  "."      # For "run_tests.h", needed by files in test-sources/
  "../inc" # For "cryptoki.h"
)

# Set QRYPTOKI_PATH macro for run_tests.cpp
target_compile_definitions(run_tests PRIVATE QRYPTOKI_PATH="${PROJECT_SOURCE_DIR}/../package/lib/libqryptoki.so")

# For dlopen, etc.
target_link_libraries(run_tests PRIVATE ${CMAKE_DL_LIBS})

# For <thread>
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(run_tests PRIVATE Threads::Threads)
