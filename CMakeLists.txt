cmake_minimum_required(VERSION 2.8.11)
project(EdisenseComms)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

find_package(Threads REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Protobuf REQUIRED)

set(ZMQPP_BUILD_STATIC false)
set(ZEROMQ_LIB_DIR  ${ZeroMQ_LIBRARIES})
set(ZEROMQ_INCLUDE_DIR ${ZeroMQ_INCLUDE_DIRS})

# Hack to get ZeroMQPP to compile.  The library doesn't actually exist on the system
set(ZEROMQ_LIBRARY_STATIC "")

add_subdirectory(ext/zmqpp)

include_directories(ext/zmqpp/src)
include_directories(src include)

set(SOURCE_FILES include/edisense_comms.h
                 include/client.h
                 include/monitor.h
                 include/member.h
                 src/client.cpp
                 src/member.cpp
                 src/monitor.cpp
                 )

add_library(EdisenseComms ${SOURCE_FILES})

target_link_libraries(EdisenseComms ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(EdisenseComms ${zmqpp})



