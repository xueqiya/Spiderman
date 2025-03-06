# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.
project("ffmpeg_android_demo")

set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs) #定义变量my_lib_path，${CMAKE_SOURCE_DIR}为当前CMakeList.txt所在路径

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
include_directories("${my_lib_path}/include" }) #包含的头文件路径

link_directories("${my_lib_path}/arm64-v8a")
link_libraries(
        avcodec
        avformat
        swscale
        avutil
        avfilter
        swresample
        avdevice
)

find_library( # Sets the name of the path variable.
        log-lib
        log)

add_library(
        ffmpeg
        SHARED
        com_xueqiya_spiderman_FFmpeg.c
)

target_link_libraries( # Specifies the target library.
        ffmpeg
        avcodec
        avformat
        swscale
        avutil
        avfilter
        swresample
        avdevice
        ${log-lib})

