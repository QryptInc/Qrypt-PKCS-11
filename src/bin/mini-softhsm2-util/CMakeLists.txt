set(INCLUDE_DIRS "common"
                 "../../../inc"
)

set(SOURCES softhsm2-util.cpp
            common/findslot.cpp
            common/getpw.cpp
            common/library.cpp
            )

include_directories(${INCLUDE_DIRS})
add_executable(mini-softhsm2-util ${SOURCES})

set_target_properties(mini-softhsm2-util PROPERTIES EXCLUDE_FROM_ALL TRUE)

target_compile_definitions(mini-softhsm2-util PRIVATE MIN_PIN_LEN=4)
target_compile_definitions(mini-softhsm2-util PRIVATE MAX_PIN_LEN=255)

target_link_libraries(mini-softhsm2-util ${CMAKE_DL_LIBS})
