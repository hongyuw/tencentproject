cmake_minimum_required(VERSION 2.8)
project(Server)
add_definitions(-std=c++11)
add_definitions(-static-libgcc -static-libstdc++)


find_package (Threads)
include_directories(${PROJECT_SOURCE_DIR}/server/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/server/src SRCS)

add_executable(Server server/src/server.cpp ${SRCS} ${PROJECT_SOURCE_DIR}/etc/config.h)
target_link_libraries(Server ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(client)