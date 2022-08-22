add_library(qryptoki SHARED BaseHSM.cpp RandomBuffer.cpp log.cpp osmutex.cpp GlobalData.cpp MeteringClientWrapper.cpp main.cpp)

target_include_directories(qryptoki PRIVATE "../inc")

target_include_directories(qryptoki PRIVATE "../deps/qrypt/MeteringClient/build/include")
target_link_libraries(qryptoki INTERFACE curl PRIVATE
    dl
    "${PROJECT_SOURCE_DIR}/deps/qrypt/MeteringClient/build/lib/libMeteringClient.a")