cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD          14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

project(qryptoki CXX)

# Set debug build

# set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type" FORCE)
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Setting compilier settings to debug")
    add_compile_options(-g)
    add_definitions(-D_DEBUG)
endif()

add_subdirectory(src)
add_subdirectory(gtests)

#========
# Install
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/build/package" CACHE PATH "default install path" FORCE)

install(TARGETS qryptoki LIBRARY DESTINATION ${INSTALL_LIB_PATH})
install(FILES inc/cryptoki.h inc/pkcs11.h DESTINATION inc)
