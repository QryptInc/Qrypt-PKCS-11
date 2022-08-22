set(BINARY ${LIBRARYNAME}lib_tst)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# avoid glob due to suggestion from CMAKE
set(TEST_SOURCES main.cpp test-httpgetter.cpp test-version.cpp)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

target_include_directories(${BINARY} PUBLIC
  ${LIBRARY_HEADERS}
  ${GTest_INCLUDE_DIRS})
target_link_libraries(${BINARY} PUBLIC
  ${LIBRARYNAME}
  GTest::gtest
  GTest::gmock)
