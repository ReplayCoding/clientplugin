cmake_policy(SET CMP0135 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

function(FetchContent_MakeAvailable_EXCLUDED name)
  FetchContent_GetProperties(${name})
  if(NOT "${name}_POPULATED")
    FetchContent_Populate(${name})
    add_subdirectory(${${name}_SOURCE_DIR} ${${name}_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endfunction()

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

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_USE_SYSTEM_INCLUDES ON)
FetchContent_Declare(
  abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp
  GIT_TAG 522606b7fae37836c138e83f6eec0eabb9947dc0
)

set(TRACY_NO_CRASH_HANDLER ON)
set(TRACY_NO_CALLSTACK ON)
set(TRACY_NO_CALLSTACK_INLINES ON)
set(TRACY_NO_SAMPLING ON)
set(TRACY_NO_SYSTEM_TRACING ON)
set(TRACY_ON_DEMAND ON)
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy
  GIT_TAG d5191ccbd4e05b2a13e00db651476518bf9b116a
)

FetchContent_Declare(
  range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3
  GIT_TAG c097c0e8d1378d95763426f024fbfeea745b4093
)

FetchContent_MakeAvailable_EXCLUDED(range-v3)
FetchContent_MakeAvailable_EXCLUDED(tracy)
FetchContent_MakeAvailable(abseil-cpp)

add_subdirectory(source-sdk)
