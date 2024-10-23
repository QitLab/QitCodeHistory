cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB appCPP *.cpp)
add_library(
        native-lib
        SHARED
        ${appCPP}
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/${CMAKE_ANDROID_ARCH_ABI}")

find_library(log-lib log)
target_link_libraries(
        native-lib
        -Wl,--start-group # 因为导入存在相互依赖，所以需要按依赖顺序导入，此标签可以忽略顺序
        avcodec avfilter avformat avutil swresample swscale
        ${log-lib}
)