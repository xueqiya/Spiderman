cmake_minimum_required(VERSION 3.22.1)

project("ffmpeg_android")

set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
include_directories("${CMAKE_SOURCE_DIR}/include")

link_directories("${my_lib_path}/arm64-v8a")

find_library(
        log-lib
        log)

add_library(
        ffmpeg
        SHARED
        com_xueqiya_spiderman_FFmpeg.c
)

target_link_libraries(
        ffmpeg
        avcodec
        avformat
        swscale
        avutil
        avfilter
        swresample
        avdevice
        ${log-lib})

