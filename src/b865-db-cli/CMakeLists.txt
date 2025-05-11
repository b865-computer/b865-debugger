file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(b865-cli ${SRC_FILES})

target_compile_options(b865-cli  PUBLIC -O3)

target_include_directories(b865-cli PUBLIC .)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(b865-cli
    b865-debugger
)