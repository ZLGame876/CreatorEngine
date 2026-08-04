@echo off
setlocal
call "D:\VisualStudio2026\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "ROOT=%~dp0.."
set "SRCFILE=%~1"
if "%SRCFILE%"=="" set "SRCFILE=%ROOT%\source\main.cpp"
set "EXT=%~x1"
if /I not "%EXT%"==".cpp" if /I not "%EXT%"==".cc" if /I not "%EXT%"==".cxx" set "SRCFILE=%ROOT%\source\main.cpp"
if not exist "%ROOT%\Bin" mkdir "%ROOT%\Bin"
cl /nologo /EHsc /std:c++17 /utf-8 /Zi /I"%ROOT%\ThirdParty\include" /I"%ROOT%\source" /I"%ROOT%\engine\source" "%SRCFILE%" "%ROOT%\source\Game.cpp" "%ROOT%\engine\source\Application.cpp" "%ROOT%\engine\source\CreatorEngine.cpp" "%ROOT%\engine\source\input\InputManager.cpp" "%ROOT%\engine\source\graphics\shaderprogram.cpp" /Fe:"%ROOT%\Bin\main.exe" /link /LIBPATH:"%ROOT%\ThirdParty\lib" glew32.lib glfw3dll.lib opengl32.lib
if errorlevel 1 exit /b %errorlevel%
copy /y "%ROOT%\ThirdParty\bin\glew32.dll" "%ROOT%\Bin" >nul
copy /y "%ROOT%\ThirdParty\bin\glfw3.dll" "%ROOT%\Bin" >nul
exit /b 0