cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB appCPP *.cpp)
add_library(
        native-lib
        SHARED
        ${appCPP}
)

# 导入静态库
add_library(getndk-lib STATIC IMPORTED)
# 开始真正导入

set_target_properties(getndk-lib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libgetndk-lib.a)


find_library(log-lib log)
target_link_libraries(
        native-lib
        ${log-lib}
        getndk-lib
)