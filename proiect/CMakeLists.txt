cmake_minimum_required(VERSION 4.2)
project(proiect C)

set(CMAKE_C_STANDARD 11)

add_executable(proiect main.c
        functions.c
        functions.h)
