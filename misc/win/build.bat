@echo off

set FILES_TO_COMPILE=%TEARA_HOME%Core\WinMain.cpp %TEARA_HOME%Utils\AssetsLoader.cpp %TEARA_HOME%3rdparty\stb\stb_image_implementation.cpp %TEARA_HOME%Utils\AudioLoader.cpp %TEARA_HOME%3rdparty\fastobj\fast_obj.cpp %TEARA_HOME%Audio\OpenALSoft\OpenALAudioSystem.cpp %TEARA_HOME%Rendering\OpenGL\TGL.cpp %TEARA_HOME%3rdparty\ufbx\ufbx.c %TEARA_HOME%3rdparty\cgltf\cgltf.cpp %TEARA_HOME%Assets\GltfLoader.cpp
set COMMON_LINK_LIBRARIES=user32.lib ole32.lib shell32.lib gdi32.lib version.lib winmm.lib advapi32.lib imm32.lib oleAut32.lib setupapi.lib opengl32.lib OpenAL32.lib E:/Engine/vcpkg/installed/x64-windows/debug/lib/assimp-vc143-mtd.lib imguid.lib stc.lib
set BUILD_LOG_FILE=build.log

if not exist %TEARA_HOME%build mkdir %TEARA_HOME%build
pushd %TEARA_HOME%build

if exist %BUILD_LOG_FILE% del %BUILD_LOG_FILE%

cl /D TEARA_DEBUG /Wall /Zi /Fm /GR- /I %TEARA_HOME% /I %VCPKG_INCLUDE% /I %VCPKG_STATIC_INCLUDE% /I "E:/Engine/vcpkg/installed/x64-windows/include/" %FILES_TO_COMPILE% /link /LIBPATH:%VCPKG_DEBUG_LIB% /LIBPATH:%VCPKG_DEBUG_BINARY% /LIBPATH:%VCPKG_STATIC_DEBUG_LIB% %COMMON_LINK_LIBRARIES% >> %BUILD_LOG_FILE%

findstr /C:"error" %BUILD_LOG_FILE%

popd