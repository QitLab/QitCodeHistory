cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB appCPP *.cpp)
add_library(
        native-lib
        SHARED
        ${appCPP}
)

# 添加动态库的路径
link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/arm64-v8a)
# 将动态库链接到目标库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/arm64-v8a -lgetndk-lib")

find_library(log-lib log)
# 添加要链接的目标库
target_link_libraries(
        native-lib
        ${log-lib}
        getndk-lib
)