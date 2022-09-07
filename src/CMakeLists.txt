add_library(qryptoki SHARED
    metering-client/AccessToken.cpp
    metering-client/base64.cpp
    metering-client/CommonUtils.cpp
    metering-client/CurlWrapper.cpp
    metering-client/HttpRandomGetter.cpp
    metering-client/RandomGetter.cpp
    BaseHSM.cpp
    RandomBuffer.cpp
    log.cpp
    osmutex.cpp
    GlobalData.cpp
    MeteringClientWrapper.cpp
    main.cpp)

target_include_directories(qryptoki PUBLIC "../inc" "metering-client")

target_link_libraries(qryptoki PUBLIC dl curl)
