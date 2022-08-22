cmake_minimum_required(VERSION 3.19)
project(QryptokiTest LANGUAGES CXX)

find_package(GTest REQUIRED)

# Name of test binary
set(BINARY ${CMAKE_PROJECT_NAME}_gtests)

# Includes
set(QRYPTOKI_TEST_PRIVATE_INC_DIRS
    "../inc"    # for pkcs11.h
    "../src"    # for RandomCollector.h
    ${GTest_INCLUDE_DIRS}  # for gtest/gtest.h
    ${GMock_INCLUDE_DIRS}  # for gmock/gmock.h
)

# Sources
set(TEST_SOURCES 
    common.cpp
    InitializeTests.cpp
    FinalizeTests.cpp
    GetInfoTests.cpp
    SeedRandomTests.cpp
    GenerateRandomTests.cpp
    BufferTests.cpp)

add_executable(${BINARY} ${TEST_SOURCES})
target_include_directories(${BINARY} PRIVATE ${QRYPTOKI_TEST_PRIVATE_INC_DIRS})
target_link_libraries(${BINARY} PRIVATE
    qryptoki
    GTest::gtest
    GTest::gmock
    gtest_main
)