cmake_minimum_required(VERSION 3.19)

project(metering_client LANGUAGES CXX VERSION 1.1.0 DESCRIPTION "Metering Client Library")

find_package(CURL      REQUIRED)
find_package(OpenSSL   REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GTest     REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LIBRARYNAME MeteringClient)
set(LIBRARY_HEADERS "${CMAKE_SOURCE_DIR}/include/MeteringClient/RandomGetter.h"
	"${CMAKE_SOURCE_DIR}/include/MeteringClient/HttpRandomGetter.h"
	"${CMAKE_SOURCE_DIR}/include/MeteringClient/ImplPtrTemplate_h.h")
	
# Enable fPIC for Linux?
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

add_subdirectory(src)

enable_testing()
add_subdirectory(gtests)