@echo off

call C:\"Program Files"\"Microsoft Visual Studio"\2022\Community\VC\Auxiliary\Build\vcvarsall.bat x64
set path=E:\Engine\TEARA\TLib;E:\Engine\TEARA\TLib\3rdparty;E:\Engine\TEARA\TCore;E:\Engine\TEARA\TGame;E:\Engine\TEARA\TCore\misc\win\;E:\Engine\SDL\SDL\VisualC\x64\Debug;E:\Engine\SDL\SDL\include;%path%