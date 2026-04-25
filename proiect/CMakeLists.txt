cmake_minimum_required(VERSION 3.28)
project(city_manager C)

set(CMAKE_C_STANDARD 11)

add_executable(city_manager
        main.c
        functions.c
        functions.h)

set_target_properties(city_manager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
