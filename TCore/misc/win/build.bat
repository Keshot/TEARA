@echo off

mkdir E:\Engine\TEARA\TCore\build
pushd E:\Engine\TEARA\TCore\build

cl /D TEARA_DEBUG /Wall /Zi /Fm /I E:/Engine/TEARA/ /I %VCPKG_INCLUDE% E:/Engine/TEARA/TCore/WinMain.cpp E:/Engine/TEARA/TLib/Utils/AssetsLoader.cpp E:/Engine/TEARA/TLib/3rdparty/stb/stb_image_implementation.cpp E:/Engine/TEARA/TLib/3rdparty/dr_wav/dr_wav_impl.cpp E:/Engine/TEARA/TLib/3rdparty/fastobj/fast_obj.cpp E:/Engine/TEARA/TCore/OpenGL/WinOGL.cpp /link /LIBPATH:"E:\vcpkg\vcpkg\installed\x64-windows\debug\lib" user32.lib ole32.lib shell32.lib gdi32.lib version.lib winmm.lib advapi32.lib imm32.lib oleAut32.lib setupapi.lib opengl32.lib OpenAL32.lib

popd