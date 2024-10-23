cmake_minimum_required(VERSION 3.22.1)
project(learning)

file(GLOB SOURCE *.c)

add_library(
        getndk-lib
        STATIC
        ${SOURCE}
)