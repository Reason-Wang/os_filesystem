cmake_minimum_required(VERSION 3.21)
project(os_filesystem C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(os_filesystem
        file.c
        file.h
        main.c
        sh.c
        sh.h)
