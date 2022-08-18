set(BINARY ${CMAKE_PROJECT_NAME}_tst)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# avoid glob due to suggestion from CMAKE
set(TEST_SOURCES main.cpp test-httpgetterimpl.cpp test-accesstoken.cpp test-commonutils.cpp)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

target_include_directories(${BINARY} PUBLIC
    ${CMAKE_INCLUDE_DIR}
    ${GTest_INCLUDE_DIRS}
    "../../include/MeteringClient"
    "../../src")
target_link_libraries(${BINARY} PUBLIC
    ${LIBRARYNAME}
    GTest::gtest
    GTest::gmock
    curl
    ssl)
