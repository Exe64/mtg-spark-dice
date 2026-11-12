# ESP-IDF firmware project. Host-side tests live in host/ (separate CMake tree).
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS core profiles ui)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sparkdice)
