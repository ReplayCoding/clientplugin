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


# WARNING: These are cached so you need to delete the build dir to change
# these, or you can change them manually
option(TRACY_NO_CRASH_HANDLER "" ON)
option(TRACY_NO_CALLSTACK "" ON)
option(TRACY_NO_CALLSTACK_INLINES "" ON)
option(TRACY_NO_SAMPLING "" ON)
option(TRACY_NO_SYSTEM_TRACING "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory(tracy)

option(ABSL_PROPAGATE_CXX_STD "" ON)
option(ABSL_USE_SYSTEM_INCLUDES "" ON)
add_subdirectory(abseil-cpp)

add_subdirectory(marl)
add_subdirectory("source-sdk")
