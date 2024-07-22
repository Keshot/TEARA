@echo off

call C:\"Program Files"\"Microsoft Visual Studio"\2022\Community\VC\Auxiliary\Build\vcvarsall.bat x64
set path=E:\Engine\TEARA\TEARA_Core;E:\Engine\TEARA\TEARA_Math;E:\Engine\TEARA\TEARA_Utils;E:\Engine\TEARA\TEARA_Misc\Win\;E:\Engine\SDL\SDL\VisualC\x64\Debug;E:\Engine\SDL\SDL\include;%path%