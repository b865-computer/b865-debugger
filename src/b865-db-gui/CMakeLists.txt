file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(b865-gui ${SRC_FILES})

target_compile_options(b865-gui  PUBLIC -O3)

target_include_directories(b865-gui PUBLIC . ../../lib/stb_image)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(b865-gui
    imgui
    ImGuiColorTextEdit
    ImGuiFileDialog
    OpenGL::GL
    glfw
    -ljsoncpp
    b865-debugger
)