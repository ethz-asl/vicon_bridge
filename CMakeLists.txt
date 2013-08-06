cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

rosbuild_genmsg()
rosbuild_gensrv()

# Generate dynamic parameters
rosbuild_find_ros_package(dynamic_reconfigure)
include(${dynamic_reconfigure_PACKAGE_PATH}/cmake/cfgbuild.cmake)
gencfg()

# check for platform
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(VICON_SDK             vicon_sdk/vicon_datastream_SDK_1.2.59611_x64)
    set(VICON_SDK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/${VICON_SDK})
    set(VICON_SDK_LINK_DIR    ${PROJECT_SOURCE_DIR}/${VICON_SDK})
elseif(CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(VICON_SDK             vicon_sdk/vicon_datastream_SDK_1.2.59611_x86)
    set(VICON_SDK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/${VICON_SDK})
    set(VICON_SDK_LINK_DIR    ${PROJECT_SOURCE_DIR}/${VICON_SDK})
else(CMAKE_SIZEOF_VOID_P MATCHES "8")
    message(FATAL_ERROR "unable to detect platform")
endif (CMAKE_SIZEOF_VOID_P MATCHES "8")

include_directories(${INCLUDE_DIRECTORIES} ${VICON_SDK_INCLUDE_DIR})
link_directories(${LINK_DIRECTORIES} ${PROJECT_SOURCE_DIR}/lib ${VICON_SDK_LINK_DIR})

rosbuild_add_boost_directories()

rosbuild_add_library(msvc_bridge src/msvc_bridge.cpp)

rosbuild_add_executable(vicon_bridge src/vicon_bridge.cpp)
target_link_libraries(vicon_bridge msvc_bridge ViconDataStreamSDK_CPP)
rosbuild_link_boost(vicon_bridge signals)

rosbuild_add_executable(calibrate src/calibrate_segment.cpp)

rosbuild_add_executable(tf_distort src/tf_distort.cpp)
rosbuild_link_boost(tf_distort signals)

rosbuild_add_executable(testclient src/ViconDataStreamSDK_CPPTest.cpp)
target_link_libraries(testclient ViconDataStreamSDK_CPP)
rosbuild_link_boost(testclient signals)
