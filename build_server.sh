#!/bin/bash

rm -rf Server
cmake .
make
sleep 1
rm -rf CMakeCache.txt cmake_install.cmake CMakeFiles Makefile