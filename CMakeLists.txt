# ESP-IDF firmware project. Host-side tests live in host/ (separate CMake tree).
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS core profiles ui)
set(PROJECT_VER "0.2.0") # shown in the device settings, next to the build date
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(sparkdice)
