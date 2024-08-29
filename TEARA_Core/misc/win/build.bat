@echo off

mkdir E:\Engine\TEARA\TEARA_Core\build
pushd E:\Engine\TEARA\TEARA_Core\build

cl /Wall /Zi /Fm /I E:/Engine/TEARA/TEARA_3RDPARTY/ /I E:/Engine/TEARA/TEARA_Core/ /I E:/Engine/TEARA/TEARA_Game/ /I E:/Engine/TEARA/TEARA_Lib/ E:/Engine/TEARA/TEARA_Core/win_main.cpp E:/Engine/TEARA/TEARA_3RDPARTY/stb/stb_image_implementation.cpp E:/Engine/TEARA/TEARA_Core/OpenGL/WinOpenGL.cpp user32.lib ole32.lib shell32.lib gdi32.lib version.lib winmm.lib advapi32.lib imm32.lib oleAut32.lib setupapi.lib opengl32.lib

popd