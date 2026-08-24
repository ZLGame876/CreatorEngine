@echo off
setlocal
call "D:\VisualStudio2026\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "ROOT=%~dp0.."
set "SRCFILE=%~1"
if "%SRCFILE%"=="" set "SRCFILE=%ROOT%\source\main.cpp"
set "EXT=%~x1"
if /I not "%EXT%"==".cpp" if /I not "%EXT%"==".cc" if /I not "%EXT%"==".cxx" set "SRCFILE=%ROOT%\source\main.cpp"
if not exist "%ROOT%\Bin" mkdir "%ROOT%\Bin"
cl /nologo /EHsc /std:c++17 /utf-8 /Zi /FS /DWIN32_LEAN_AND_MEAN /I"%ROOT%\ThirdParty" /I"%ROOT%\ThirdParty\include" /I"%ROOT%\ThirdParty\imgui" /I"%ROOT%\ThirdParty\imgui\backends" /I"%ROOT%\source" /I"%ROOT%\engine\source" "%SRCFILE%" "%ROOT%\source\Game.cpp" "%ROOT%\engine\source\Application.cpp" "%ROOT%\engine\source\CreatorEngine.cpp" "%ROOT%\engine\source\input\InputManager.cpp" "%ROOT%\engine\source\graphics\shaderprogram.cpp" "%ROOT%\engine\source\graphics\Texture.cpp" "%ROOT%\engine\source\graphics\Framebuffer.cpp" "%ROOT%\engine\source\graphics\SpriteRenderer.cpp" "%ROOT%\engine\source\graphics\SpriteBatch.cpp" "%ROOT%\engine\source\graphics\Camera.cpp" "%ROOT%\engine\source\core\Component.cpp" "%ROOT%\engine\source\core\Transform.cpp" "%ROOT%\engine\source\core\GameObject.cpp" "%ROOT%\engine\source\core\Scene.cpp" "%ROOT%\engine\source\core\SceneSerializer.cpp" "%ROOT%\engine\source\scripting\MonoRuntime.cpp" "%ROOT%\engine\source\scripting\CSharpScript.cpp" "%ROOT%\engine\source\scripting\NativeHandleRegistry.cpp" "%ROOT%\engine\source\editor\Editor.cpp" "%ROOT%\engine\source\physics\Rigidbody2D.cpp" "%ROOT%\engine\source\physics\PhysicsMaterial.cpp" "%ROOT%\engine\source\physics\Collider2D.cpp" "%ROOT%\engine\source\physics\BoxCollider2D.cpp" "%ROOT%\engine\source\physics\CircleCollider2D.cpp" "%ROOT%\engine\source\physics\PhysicsWorld.cpp" "%ROOT%\ThirdParty\imgui\imgui.cpp" "%ROOT%\ThirdParty\imgui\imgui_draw.cpp" "%ROOT%\ThirdParty\imgui\imgui_tables.cpp" "%ROOT%\ThirdParty\imgui\imgui_widgets.cpp" "%ROOT%\ThirdParty\imgui\imgui_demo.cpp" "%ROOT%\ThirdParty\imgui\backends\imgui_impl_glfw.cpp" "%ROOT%\ThirdParty\imgui\backends\imgui_impl_opengl3.cpp" /Fe:"%ROOT%\Bin\main.exe" /link /LIBPATH:"%ROOT%\ThirdParty\lib" glew32.lib glfw3dll.lib opengl32.lib
if errorlevel 1 exit /b %errorlevel%
copy /y "%ROOT%\ThirdParty\bin\glew32.dll" "%ROOT%\Bin" >nul
copy /y "%ROOT%\ThirdParty\bin\glfw3.dll" "%ROOT%\Bin" >nul
exit /b 0
