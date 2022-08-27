cmake_minimum_required(VERSION 3.10)

# mimic source engine here
cmake_policy(SET CMP0072 OLD)

# uggghghhhh why doesn't vcpkg have a x86-linux triplet?
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR})
set(VCPKG_TARGET_TRIPLET x86-linux-mixed)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "vcpkg toolchain file")

project("plugin")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("external")
add_subdirectory("src")
