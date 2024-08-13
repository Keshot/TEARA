@echo off

mkdir E:\Engine\TEARA\TEARA_Core\build
pushd E:\Engine\TEARA\TEARA_Core\build

cl /Wall /Zi /Fm /I E:/Engine/TEARA/TEARA_3RDPARTY/ /I E:/Engine/TEARA/TEARA_Rendering/ /I E:/Engine/TEARA/TEARA_Math/ /I E:/Engine/SDL/SDL/include/ /I E:/Engine/TEARA/TEARA_Utils/ E:/Engine/TEARA/TEARA_Core/main.cpp E:/Engine/TEARA/TEARA_3RDPARTY/stb/stb_image_implementation.cpp E:/Engine/TEARA/TEARA_Rendering/OpenGL/OGL.cpp E:/Engine/SDL/SDL/VisualC/x64/Debug/SDL3.lib user32.lib ole32.lib shell32.lib gdi32.lib version.lib winmm.lib advapi32.lib imm32.lib oleAut32.lib setupapi.lib opengl32.lib

popd