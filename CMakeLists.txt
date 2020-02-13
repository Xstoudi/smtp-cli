cmake_minimum_required(VERSION 3.15)
project(smtp-cli C)

set(CMAKE_C_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB sources
        "${PROJECT_SOURCE_DIR}/src/*.c"
        "${PROJECT_SOURCE_DIR}/include/*.h")

add_executable(smtp-cli ${sources})
