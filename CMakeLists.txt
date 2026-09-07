cmake_minimum_required(VERSION 4.3)
project(just_a_db C)

set(CMAKE_C_STANDARD 11)

# This file exists only so CLion can index/navigate the project.
# Actual builds go through the Makefile (make / make sanitize / make test).
file(GLOB SOURCES src/*.c)
add_executable(just_a_db ${SOURCES})
target_include_directories(just_a_db PRIVATE include)

add_executable(test_disk_manager_roundtrip
        src/disk_manager.c
        test/test_disk_manager_roundtrip.c)
target_include_directories(test_disk_manager_roundtrip PRIVATE include)

add_executable(test_disk_manager_bounds
        src/disk_manager.c
        test/test_disk_manager_bounds.c)
target_include_directories(test_disk_manager_bounds PRIVATE include)
