cmake_minimum_required(VERSION 2.8.3)
project(vicon_bridge)

find_package(catkin REQUIRED COMPONENTS
    message_generation
    dynamic_reconfigure
    geometry_msgs
    roscpp
    tf
    diagnostic_updater
)

find_package(Boost REQUIRED COMPONENTS thread)

# Generate messages and services
add_message_files(FILES
    Marker.msg
    Markers.msg
    TfDistortInfo.msg
)

add_service_files(FILES
    viconCalibrateSegment.srv
    viconGrabPose.srv
)

generate_messages(DEPENDENCIES geometry_msgs)

# Generate dynamic parameters
generate_dynamic_reconfigure_options(
  cfg/tf_distort.cfg
)

catkin_package(CATKIN_DEPENDS 
    dynamic_reconfigure
    geometry_msgs
    message_runtime 
    roscpp 
)

# get CPU and OS architecture
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(ARCHI 64)
    SET(ARCHI2 64)
elseif(CMAKE_SIZEOF_VOID_P MATCHES "4")
    SET(ARCHI 32)
    SET(ARCHI2 86)
else()
    message(FATAL_ERROR "unable to detect platform")
endif()
message(STATUS "OS " ${ARCHI} " bits detected")

# check for platform and use the right Vicon SDK
set(VICON_SDK_VERSION "1.7")

if(VICON_SDK_VERSION MATCHES "1.8")
    set(VICON_SDK vicon_sdk/ViconDataStreamSDK_1.8.0_105615h/Linux${ARCHI}/)
    message(STATUS "vicon sdk version is 1.8")

elseif(VICON_SDK_VERSION MATCHES "1.3")
    set(VICON_SDK vicon_sdk/Vicon_SDK_1.3_Linux/${ARCHI}-bit/)
    message(STATUS "vicon sdk version is 1.3")

elseif(VICON_SDK_VERSION MATCHES "1.7")
    set(VICON_SDK vicon_sdk/ViconDataStreamSDK_1.7.1_96542h/Linux${ARCHI}-boost-1.58.0/20170125_96542h.x${ARCHI2})
    message(STATUS "vicon sdk version is 1.7")

else()
    message(FATAL_ERROR "Vicon version "${VICON_SDK_VERSION}" not found")

endif()

# Setup the library and include files for the sdk
set(VICON_SDK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/${VICON_SDK})
set(VICON_SDK_LINK_DIR    ${PROJECT_SOURCE_DIR}/${VICON_SDK})
find_library(VICON_SDK_LIBRARY ViconDataStreamSDK_CPP PATHS ${VICON_SDK_LINK_DIR})

include_directories(src ${catkin_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${VICON_SDK_INCLUDE_DIR})

add_library(msvc_bridge src/msvc_bridge.cpp)

add_executable(vicon_bridge src/vicon_bridge.cpp)
target_link_libraries(vicon_bridge
    msvc_bridge
    ${VICON_SDK_LIBRARY}
    ${catkin_LIBRARIES}
)
add_dependencies(vicon_bridge ${PROJECT_NAME}_gencpp)

add_executable(calibrate src/calibrate_segment.cpp)
target_link_libraries(calibrate
   ${catkin_LIBRARIES}
)
add_dependencies(calibrate ${PROJECT_NAME}_gencpp)

add_executable(tf_distort src/tf_distort.cpp)
target_link_libraries(tf_distort
   ${catkin_LIBRARIES}
   ${Boost_LIBRARIES}
)
add_dependencies(tf_distort ${PROJECT_NAME}_gencpp ${PROJECT_NAME}_gencfg)

add_executable(testclient src/ViconDataStreamSDK_CPPTest.cpp)
target_link_libraries(testclient 
    ${VICON_SDK_LIBRARY}
    ${Boost_LIBRARIES}
)
