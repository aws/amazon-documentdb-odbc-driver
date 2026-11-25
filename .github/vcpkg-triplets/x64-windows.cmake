# CI overlay: build vcpkg ports with the v142 toolset (MSVC 14.29).
# Boost 1.81 (pinned by the vcpkg baseline) cannot build with MSVC 14.4x.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PLATFORM_TOOLSET v142)
