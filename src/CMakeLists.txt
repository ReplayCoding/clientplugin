add_library(
  clientplugin SHARED

  interfaces.cpp
  plugin.cpp
  util/mmap.cpp
  util/loaded_module.cpp
  offsets/offsets.cpp
  offsets/eh_frame.cpp
  offsets/rtti.cpp
  offsets/clientclasses.cpp
  modules/modules.cpp
  modules/unhidecvars.cpp
  modules/gfxoverlay.cpp
  modules/mediguns.cpp
  modules/nav_workshop_fix.cpp
  modules/profiler.cpp
  modules/cleaverbug.cpp
  modules/allow_login_nosteam.cpp
  modules/partychat.cpp
  hook/gum/interceptor.cpp
  hook/gum/x86patcher.cpp
)

target_compile_features(clientplugin PUBLIC c_std_99 cxx_std_20)

target_compile_options(
  clientplugin PRIVATE
  -Wall -Wextra -Wpedantic -Werror
)

target_include_directories(
  clientplugin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(marl CONFIG REQUIRED)

target_link_options(
  clientplugin PRIVATE
  LINKER:--no-undefined
)
target_link_libraries(
  clientplugin PRIVATE
  sdk::tier0
  sdk::tier1
  sdk::tier2
  sdk::tier3
  sdk::bitmap

  fmt::fmt
  OpenGL::GL
  imgui::imgui
  absl::numeric
  absl::btree
  absl::flat_hash_set
  absl::flat_hash_map
  absl::inlined_vector
  range-v3::range-v3
  elfio::elfio
  Tracy::TracyClient
  SDL2::SDL2
  marl::marl
  Boost::algorithm

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
