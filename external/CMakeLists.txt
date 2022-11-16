# TODO: Make the frida stuff into a Find*.cmake file

cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare(
  _GUM
  FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER
  URL https://github.com/frida/frida/releases/download/15.1.28/frida-gum-devkit-15.1.28-linux-x86.tar.xz
  URL_HASH SHA256=45276051648a12f88b6763b06b31ac30fcd3e27c22b2b70623cd5da19ad92558
)
FetchContent_MakeAvailable(_GUM)
add_library(frida-gum STATIC IMPORTED GLOBAL)
set_target_properties(frida-gum PROPERTIES IMPORTED_LOCATION ${_gum_SOURCE_DIR}/libfrida-gum.a)
target_include_directories(frida-gum INTERFACE ${_gum_SOURCE_DIR})

option(TRACY_NO_CRASH_HANDLER "" OFF)
add_subdirectory(tracy)

add_subdirectory("source-sdk")
