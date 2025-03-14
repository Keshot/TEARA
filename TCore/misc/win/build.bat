@echo off

IF NOT EXIST %TEARA_HOME%TCore\build mkdir %TEARA_HOME%TCore\build
pushd %TEARA_HOME%TCore\build

set FILES_TO_COMPILE=%TEARA_HOME%TCore\WinMain.cpp %TEARA_HOME%TLib\Utils\AssetsLoader.cpp %TEARA_HOME%TLib\3rdparty\stb\stb_image_implementation.cpp %TEARA_HOME%TLib\Utils\AudioLoader.cpp %TEARA_HOME%TLib\3rdparty\fastobj\fast_obj.cpp %TEARA_HOME%TCore\OpenGL\WinOGL.cpp %TEARA_HOME%TLib\Audio\OpenALSoft\OpenALAudioSystem.cpp
set COMMON_LINK_LIBRARIES=user32.lib ole32.lib shell32.lib gdi32.lib version.lib winmm.lib advapi32.lib imm32.lib oleAut32.lib setupapi.lib opengl32.lib OpenAL32.lib

echo %FILES_TO_COMPILE%

cl /D TEARA_DEBUG /Wall /Zi /Fm /GR- /I %TEARA_HOME% /I %VCPKG_INCLUDE% %FILES_TO_COMPILE% /link /LIBPATH:%VCPKG_DEBUG_LIB% /LIBPATH:%VCPKG_DEBUG_LIB% %COMMON_LINK_LIBRARIES% >> build.log

popd