cmake_policy(SET CMP0135 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

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

set(ABSL_PROPAGATE_CXX_STD "" ON)
set(ABSL_USE_SYSTEM_INCLUDES "" ON)
FetchContent_Declare(
  abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp
  GIT_TAG 522606b7fae37836c138e83f6eec0eabb9947dc0
)
FetchContent_MakeAvailable(abseil-cpp)

set(TRACY_NO_CRASH_HANDLER ON)
set(TRACY_NO_CALLSTACK ON)
set(TRACY_NO_CALLSTACK_INLINES ON)
set(TRACY_NO_SAMPLING ON)
set(TRACY_NO_SYSTEM_TRACING ON)
set(TRACY_ON_DEMAND ON)
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/ReplayCoding/tracy
  GIT_TAG 1480590824c0006e465f41da80232a87325f4364
)
FetchContent_MakeAvailable(tracy)

add_subdirectory(source-sdk)
