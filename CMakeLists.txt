cmake_minimum_required(VERSION 3.10)
project("plugin")

set(CMAKE_C_STANDARD_REQUIRED 99)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("external")
add_subdirectory("src")
