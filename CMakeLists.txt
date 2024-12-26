cmake_minimum_required(VERSION 3.22)
project(affinity)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(affinity INTERFACE)
target_include_directories(affinity INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(tests tests/basic.cpp)
target_link_libraries(tests PRIVATE affinity)

target_compile_options(tests PRIVATE -Wall -Wextra -Werror)
target_link_options(tests PRIVATE -fsanitize=address,undefined)