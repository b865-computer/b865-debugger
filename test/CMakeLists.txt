cmake_minimum_required(VERSION 3.22)

set(CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(emulator_test
    LANGUAGES CXX)

add_executable(emulator_test emulator_test.cpp ../src/CPU.cpp ../src/MEM.cpp ../src/Pheriph.cpp)

find_package(Catch2 3 REQUIRED)

target_link_libraries(emulator_test
    PRIVATE
    Catch2::Catch2WithMain)


add_custom_command(
     TARGET emulator_test
     COMMENT "Run tests"
     POST_BUILD 
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
     COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^emulator_test$" --output-on-failures
)