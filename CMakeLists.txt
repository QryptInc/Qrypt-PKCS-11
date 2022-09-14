cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD          14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

project(Qrypt-PKCS-11 LANGUAGES CXX)

find_package(CURL      REQUIRED)
find_package(OpenSSL   REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GTest     REQUIRED)

# set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type" FORCE)
set(CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Setting compilier settings to debug")
    add_compile_options(-g)
    add_definitions(-D_DEBUG)
endif()

add_subdirectory(src)

#========
# Install
# TODO: Allow for better install location, more flexibility
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/package" CACHE PATH "default install path" FORCE)

install(TARGETS qryptoki LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES inc/cryptoki.h inc/pkcs11.h inc/pkcs11f.h inc/pkcs11t.h inc/qryptoki_pkcs11_vendor_defs.h DESTINATION inc)
install(TARGETS mini-softhsm2-util DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)
