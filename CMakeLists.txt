cmake_minimum_required(VERSION 3.15)
project(b865-db)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ")

add_subdirectory(lib)
add_subdirectory(src)