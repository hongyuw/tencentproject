cmake_minimum_required(VERSION 2.8)

project(client)
add_definitions(-std=c++11)
add_definitions(-static-libgcc -static-libstdc++)

find_package (Threads)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRCS)
include_directories(${PROJECT_SOURCE_DIR}/include)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(client ${SRCS} ../etc/config.h) 

target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})