cmake_minimum_required(VERSION 3.20)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB SRC_FILES ./*.cpp ./Parser/*.cpp)

# Target library
add_library(b865-debugger STATIC ${SRC_FILES})

target_include_directories(b865-debugger PUBLIC
    .
)

target_link_libraries(b865-debugger PUBLIC 
    CdbgExpr
    libintelhex
)