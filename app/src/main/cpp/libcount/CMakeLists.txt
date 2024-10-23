cmake_minimum_required(VERSION 3.22.1)
project(learning)
file(GLOB SOURCE *.cpp)

add_library(
        count-lib
        STATIC
        ${SOURCE}
)