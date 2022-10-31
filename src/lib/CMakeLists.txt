add_library(qryptoki SHARED
    base64.cpp
    BaseHSM.cpp
    CurlWrapper.cpp
    RandomBuffer.cpp
    log.cpp
    osmutex.cpp
    GlobalData.cpp
    main.cpp)

    target_include_directories(qryptoki PUBLIC "../../inc")

    target_link_libraries(qryptoki PUBLIC dl curl)
