cmake_minimum_required(VERSION 2.8)
# project name
project(vscode_cmake_gstreamer_learn)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -lX11")

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")
MESSAGE(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM}")
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

if(${CMAKE_SYSTEM} MATCHES "Linux-4.9.140-tegra")
    message("On TEGRA PLATFORM.")
    add_definitions(-DPLATFORM_TEGRA)
    set(SYS_USR_LIB /usr/lib/aarch64-linux-gnu)
    set(SYS_LIB /lib/aarch64-linux-gnu)
    set(DS_LIB /opt/nvidia/deepstream/deepstream-4.0/lib)
else()
    # message("On X86 PLATFORM.")
    # set(SYS_USR_LIB /usr/lib/x86_64-linux-gnu)
    # set(SYS_LIB /lib/x86_64-linux-gnu)

    message("On aarch64 PLATFORM.")
    set(SYS_USR_LIB /usr/lib/aarch64-linux-gnu)
    set(SYS_LIB /lib/aarch64-linux-gnu)
endif()

# gst
include_directories(
        /usr/includes
        /usr/include/gstreamer-1.0
        /usr/lib/aarch64-linux-gnu/gstreamer-1.0
        /usr/lib/gstreamer-1.0
        /usr/include/glib-2.0
        ${SYS_USR_LIB}/glib-2.0/include
        /usr/include/orc-0.4
        /usr/include/rockchip
        /usr/local/include/opencv4/opencv2
        /usr/local/include
        /usr/local/lib/aarch64-linux-gnu
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        # ${CMAKE_CURRENT_SOURCE_DIR}/rga/librga
        # /home/teamhd/librga-main/include
        # rk3588
        /usr/local/include
        /usr/local/lib/aarch64-linux-gnu
        /usr/local/include/opencv4
)

link_libraries(
        ${SYS_USR_LIB}/libgstreamer-1.0.so.0
        ${SYS_USR_LIB}/libgobject-2.0.so.0
        ${SYS_USR_LIB}/libglib-2.0.so.0
        ${SYS_USR_LIB}/libgstbase-1.0.so.0
	    ${SYS_USR_LIB}/libgstvideo-1.0.so.0
        ${SYS_USR_LIB}/libgstapp-1.0.so.0
        ${SYS_LIB}/libc.so.6
        ${SYS_LIB}/libdl.so.2
        ${SYS_LIB}/libpthread.so.0
        ${SYS_USR_LIB}/libgmodule-2.0.so.0
        ${SYS_LIB}/libm.so.6
        # rk3399prod
        # ${SYS_USR_LIB}/libffi.so.6
        ${SYS_LIB}/libpcre.so.3
        # ${CMAKE_CURRENT_SOURCE_DIR}/rga/librga/build/librga.so
        /usr/lib/aarch64-linux-gnu/librga.so
        #/home/teamhd/librga-main/libs/Linux/gcc-aarch64/librga.so
        # rk3588
        ${SYS_USR_LIB}/libffi.so
)

# debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(CURL REQUIRED)
find_package(OpenCV REQUIRED)
set(link_libs curl pthread opencv_core opencv_highgui opencv_imgcodecs)

set(SRC_LIST ./test/c_test_main_rtsp.cpp 
            ./src/gst_rtsp_client.cpp)

add_executable (main_gst_rtsp_ ${SRC_LIST})
target_link_libraries(main_gst_rtsp_ ${link_libs})
add_library(RtspClientLib SHARED ./include/gst_rtsp_client.h ./src/gst_rtsp_client.cpp ./src/setup.cpp)
