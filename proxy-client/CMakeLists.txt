cmake_minimum_required(VERSION 2.8)

project(FIRSTPROXY)

# 3.头文件
include_directories(${PROJECT_SOURCE_DIR}/include)

include_directories(${PROJECT_SOURCE_DIR}/etc)

#4.source directory，源文件目录

AUX_SOURCE_DIRECTORY(src SRCS)

SET(FIRSTPROXY${SRCS})
#设定可执行二进制文件的目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置编译选项
# 允许c++11标准、O3优化、多线程。match选项可避免一些cpu上的问题
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )

add_executable( FIRSTPROXY ${SRCS} ) # 入口为 myproxy.cpp