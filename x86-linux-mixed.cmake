set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_C_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")
set(VCPKG_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")

# SDL2 should be a shared lib. We also have to set the buildtype for SDL,
# because it will change the lib name in debug mode
if (${PORT} MATCHES "sdl2")
  set(VCPKG_BUILD_TYPE release)
  set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
