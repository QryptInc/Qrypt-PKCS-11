cmake_minimum_required(VERSION 3.19)

project(end-to-end-example CXX)

add_executable(get_random get_random.cpp)

target_include_directories(get_random PRIVATE "../inc")
target_link_libraries(get_random PRIVATE ${CMAKE_DL_LIBS})
