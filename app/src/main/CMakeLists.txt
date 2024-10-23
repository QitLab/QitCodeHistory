cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/cpp/*.cpp ${CMAKE_SOURCE_DIR}/cpp/*.c)
add_library(
        native-lib
        SHARED
        ${SOURCE}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/cpp/libgetndk)
add_subdirectory(${CMAKE_SOURCE_DIR}/cpp/libcount)

target_link_libraries(
        native-lib
        log
        getndk-lib
        count-lib
)