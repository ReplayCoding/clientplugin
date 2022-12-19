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

CPMAddPackage(
  NAME frida-gum
  DOWNLOAD_ONLY YES
  URL https://github.com/frida/frida/releases/download/16.0.8/frida-gum-devkit-16.0.8-linux-x86.tar.xz
)
if(frida-gum_ADDED)
  add_library(frida-gum STATIC IMPORTED GLOBAL)
  set_target_properties(frida-gum PROPERTIES IMPORTED_LOCATION ${frida-gum_SOURCE_DIR}/libfrida-gum.a)
  target_include_directories(frida-gum INTERFACE ${frida-gum_SOURCE_DIR})
endif()

CPMAddPackage(
  NAME abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp
  GIT_TAG d2c0c9d06128f9b63ad2f1e5165a16cb8512cdf2
  OPTIONS
    "ABSL_PROPAGATE_CXX_STD ON"
    "ABSL_USE_SYSTEM_INCLUDES ON"
)

CPMAddPackage(
  NAME tracy
  GITHUB_REPOSITORY wolfpld/tracy
  GIT_TAG b80ede3ec607c5b4d7254cf00e8f548656ed9879
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "TRACY_NO_CRASH_HANDLER ON"
    "TRACY_NO_CALLSTACK ON"
    "TRACY_NO_CALLSTACK_INLINES ON"
    "TRACY_NO_SAMPLING ON"
    "TRACY_NO_SYSTEM_TRACING ON"
    "TRACY_ON_DEMAND ON"
)

CPMAddPackage(
  NAME range-v3
  GITHUB_REPOSITORY ericniebler/range-v3
  GIT_TAG 4624c63972c6f2c2871c7b87813c42048ddb80ad
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "RANGES_NATIVE OFF"
    "RANGES_DEEP_STL_INTEGRATION ON"
    "RANGES_PREFER_REAL_CONCEPTS ON"
    "RANGES_VERBOSE_BUILD ON"
)

CPMAddPackage(
  NAME source-sdk
  GITHUB_REPOSITORY ReplayCoding/hl2sdk-cmake
  GIT_TAG 6e3028c724298c1e3c8bc780d07879779e4f8d4c
)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:serge1/ELFIO#6fc23e53ddf6109b0d9c0f3d8ed62964405369d2")
if (ELFIO_ADDED)
  target_compile_options(elfio INTERFACE -Wno-missing-field-initializers)
endif()
