cmake_minimum_required(VERSION 4.3)
project(just_a_db C)

set(CMAKE_C_STANDARD 11)

# This file exists only so CLion can index/navigate the project.
# Actual builds go through the Makefile (make / make sanitize / make test).
file(GLOB SOURCES src/*.c)
add_executable(just_a_db ${SOURCES}
        src/disk_manager.c)
target_include_directories(just_a_db PRIVATE include)
