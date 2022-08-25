add_library(
  clientplugin SHARED

  interfaces.cpp
  plugin.cpp
  offsets.cpp
  clientclasses.cpp
  rtti.cpp
  # x264encoder.cpp
  modules/modules.cpp
  modules/unhidecvars.cpp
  # modules/gfxoverlay.cpp
  # modules/videorecord.cpp
  hook/gum/interceptor.cpp
  hook/gum/x86patcher.cpp
)

target_compile_options(
  clientplugin PRIVATE
  -Wall -Wextra -Wpedantic -Werror
  -Wno-vla
)

target_include_directories(
  clientplugin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  clientplugin PRIVATE
  tier0
  tier1
  tier2
  tier3
  bitmap

  fmt::fmt
  frida-gum
)

install(
  FILES ClientPlugin.vdf
  DESTINATION addons/
)
install(
  TARGETS clientplugin
  DESTINATION addons/
)
