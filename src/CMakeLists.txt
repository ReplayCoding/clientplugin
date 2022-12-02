add_library(
  clientplugin SHARED

  interfaces.cpp
  plugin.cpp
  clientclasses.cpp
  x264encoder.cpp
  util/mmap.cpp
  offsets/offsets.cpp
  offsets/eh_frame.cpp
  offsets/rtti.cpp
  modules/modules.cpp
  modules/unhidecvars.cpp
  modules/gfxoverlay.cpp
  modules/mediguns.cpp
  modules/videorecord.cpp
  modules/profiler.cpp
  hook/gum/interceptor.cpp
  hook/gum/x86patcher.cpp
)

target_compile_features(clientplugin PUBLIC c_std_99 cxx_std_20)

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
find_package(range-v3 CONFIG REQUIRED)
find_package(elfio CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
pkg_check_modules(x264 REQUIRED IMPORTED_TARGET x264)

target_link_options(clientplugin PRIVATE LINKER:--no-undefined)
target_link_libraries(
  clientplugin PRIVATE
  sdk::tier0
  sdk::tier1
  sdk::tier2
  sdk::tier3
  sdk::bitmap

  fmt::fmt
  PkgConfig::x264
  OpenGL::GL
  imgui::imgui
  absl::numeric
  absl::flat_hash_set
  absl::flat_hash_map
  absl::inlined_vector
  range-v3::range-v3
  elfio::elfio
  marl
  Tracy::TracyClient
  SDL2::SDL2

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
