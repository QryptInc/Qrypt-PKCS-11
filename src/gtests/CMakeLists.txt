find_package(GTest REQUIRED)

# Includes
set(QRYPTOKI_TEST_PRIVATE_INC_DIRS
    "../../inc"    # for pkcs11.h
    "../lib"    # for RandomCollector.h
    ${GTest_INCLUDE_DIRS}  # for gtest/gtest.h
    ${GMock_INCLUDE_DIRS}  # for gmock/gmock.h
)

# Sources
set(TEST_SOURCES 
    base64.cpp
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