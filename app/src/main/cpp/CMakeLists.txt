cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB appCPP *.cpp)
add_library(
        native-lib
        SHARED
        ${appCPP}
)

set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp)

include_directories(${FFMPEG}/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}")

target_link_libraries(
        native-lib
        -Wl,--start-group # 因为导入存在相互依赖，所以需要按依赖顺序导入，此标签可以忽略顺序
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        log
        z #ffmpeg需要ndk的z库
        rtmp
        android #ANativeWindow 用来渲染画面
        OpenSLES # 播放声音
)