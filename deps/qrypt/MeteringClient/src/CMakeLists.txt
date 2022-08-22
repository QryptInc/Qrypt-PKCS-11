# avoid glob due to suggestion from CMAKE
set(SOURCES AccessToken.cpp CurlWrapper.cpp HttpRandomGetter.cpp RandomGetter.cpp base64.cpp CommonUtils.cpp)

# add a library for output
add_library(${LIBRARYNAME} STATIC ${SOURCES} ${LIBRARY_HEADERS})
target_compile_features(${LIBRARYNAME} PUBLIC cxx_std_14)
set_target_properties(${LIBRARYNAME} PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      PUBLIC_HEADER "${LIBRARY_HEADERS}")
target_include_directories(${LIBRARYNAME}
  SYSTEM PRIVATE
    ${RapidJSON_INCLUDE_DIRS}
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include/MeteringClient
  PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TARGET_BUILD_SPECIFIC_INCLUDE})
target_link_libraries(${LIBRARYNAME} PRIVATE
    curl
    ssl
    ${TARGET_BUILD_SPECIFIC_LIBRARIES})

# Set default install target
# TODO: Provide more flexibility?
# if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build CACHE PATH "..." FORCE)
# endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(TARGETS ${LIBRARYNAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
