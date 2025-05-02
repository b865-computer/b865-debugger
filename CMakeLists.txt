cmake_minimum_required(VERSION 3.15)
project(b865-debugger)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_executable(b865-debugger ${SRC_FILES})

target_compile_options(b865-debugger PUBLIC -O3)

target_include_directories(b865-debugger PUBLIC src lib/stb_image)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(b865-debugger
    imgui
    ImGuiColorTextEdit
    ImGuiFileDialog
    libintelhex
    OpenGL::GL
    glfw
    -ljsoncpp
    CdbgExpr
)