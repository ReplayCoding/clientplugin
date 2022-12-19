set(CPM_DOWNLOAD_VERSION 0.36.0)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

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

CPMAddPackage(
  tracy
  GITHUB_REPOSITORY wolfpld/tracy
  GIT_TAG d5191ccbd4e05b2a13e00db651476518bf9b116a
  EXCLUDE_FROM_ALL 1
  OPTIONS
    "TRACY_NO_CRASH_HANDLER ON"
    "TRACY_NO_CALLSTACK ON"
    "TRACY_NO_CALLSTACK_INLINES ON"
    "TRACY_NO_SAMPLING ON"
    "TRACY_NO_SYSTEM_TRACING ON"
    "TRACY_ON_DEMAND ON"
)

CPMAddPackage(
  range-v3
  GITHUB_REPOSITORY ericniebler/range-v3
  GIT_TAG c097c0e8d1378d95763426f024fbfeea745b4093
  EXCLUDE_FROM_ALL 1
  OPTIONS
    "RANGES_NATIVE OFF"
    "RANGES_DEEP_STL_INTEGRATION ON"
    "RANGES_PREFER_REAL_CONCEPTS ON"
    "RANGES_VERBOSE_BUILD ON"
)

FetchContent_MakeAvailable(abseil-cpp)

add_subdirectory(source-sdk)
