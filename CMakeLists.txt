cmake_minimum_required(VERSION 3.24)

# mimic source engine here
set(OpenGL_GL_PREFERENCE "LEGACY")

# uggghghhhh why doesn't vcpkg have a x86-linux triplet?
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR})
set(VCPKG_TARGET_TRIPLET x86-linux-mixed)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "vcpkg toolchain file")

project("plugin")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
 add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
 add_compile_options (-fcolor-diagnostics)
endif ()

add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory("external")
add_subdirectory("src")
