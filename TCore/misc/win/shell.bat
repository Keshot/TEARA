@echo off

call %VS_COMPILER_PATH%vcvarsall.bat x64
set path=%TEARA_MISC%;%VCPKG_DEBUG_BINARY%;%VCPKG_DEBUG_LIB%;%path%