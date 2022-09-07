cmake_minimum_required(VERSION 3.19)
project(Qrypt-PKCS-11-Test LANGUAGES CXX)

find_package(GTest REQUIRED)

# Includes
set(QRYPTOKI_TEST_PRIVATE_INC_DIRS
    "../inc"    # for pkcs11.h
    "../src"    # for RandomCollector.h
    ${GTest_INCLUDE_DIRS}  # for gtest/gtest.h
    ${GMock_INCLUDE_DIRS}  # for gmock/gmock.h
)

# Sources
set(TEST_SOURCES 
    metering-client/lib/base64.cpp
    metering-client/lib/test-httpgetter.cpp
    metering-client/lib/test-version.cpp
    metering-client/src/test-accesstoken.cpp
    metering-client/src/test-commonutils.cpp
    metering-client/src/test-httpgetterimpl.cpp
    common.cpp
    InitializeTests.cpp
    FinalizeTests.cpp
    GetFunctionListTests.cpp
    GetInfoTests.cpp
    SeedRandomTests.cpp
    GenerateRandomTests.cpp
    BufferTests.cpp)

add_executable(qryptoki_gtests ${TEST_SOURCES})
target_include_directories(qryptoki_gtests PRIVATE ${QRYPTOKI_TEST_PRIVATE_INC_DIRS})
target_link_libraries(qryptoki_gtests PRIVATE
    qryptoki
    GTest::gtest
    GTest::gmock
    gtest_main
)