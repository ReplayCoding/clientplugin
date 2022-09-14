add_library(
  clientplugin SHARED

  interfaces.cpp
  plugin.cpp
  offsets.cpp
  clientclasses.cpp
  rtti.cpp
  x264encoder.cpp
  util/mmap.cpp
  modules/modules.cpp
  modules/unhidecvars.cpp
  modules/gfxoverlay.cpp
  modules/videorecord.cpp
  hook/gum/interceptor.cpp
  hook/gum/x86patcher.cpp
)

if (CMAKE_BUILD_TYPE MATCHES Release)
  target_precompile_headers(clientplugin PRIVATE pch.hpp)
endif()

target_compile_options(
  clientplugin PRIVATE
  -Wall -Wextra -Wpedantic -Werror
  -Wno-vla # this is stupid
)

target_include_directories(
  clientplugin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
pkg_check_modules(x264 REQUIRED IMPORTED_TARGET x264)
pkg_check_modules(elfutils REQUIRED IMPORTED_TARGET libelf)

find_package(SDL2 REQUIRED)
# fmt as a lib is fucked. make sure to catch errors like this BEFORE we run
target_link_options(clientplugin PRIVATE LINKER:--no-undefined)
target_link_libraries(
  clientplugin PRIVATE
  sdk::tier0
  sdk::tier1
  sdk::tier2
  sdk::tier3
  sdk::bitmap

  fmt::fmt-header-only
  SDL2::SDL2
  PkgConfig::x264
  PkgConfig::elfutils
  OpenGL::GL
  imgui::imgui

  frida-gum
)

# Remove SDL2 rpath
add_custom_command(
  TARGET clientplugin POST_BUILD
  COMMAND patchelf --remove-rpath "$<TARGET_FILE:clientplugin>"
  VERBATIM
)

install(
  FILES ClientPlugin.vdf
  DESTINATION addons/
)
install(
  TARGETS clientplugin
  DESTINATION addons/
)
