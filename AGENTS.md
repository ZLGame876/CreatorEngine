# AGENTS.md

C++ OpenGL game engine (GLFW + GLEW, OpenGL 3.3 core). Git repo, remote `origin` = https://github.com/ZLGame876/CreatorEngine.git. Two branches: `main` (macOS config) and `windows` (Windows config).

## Dual-platform git workflow

- `main` = macOS, `windows` = Windows; switch machines with `git checkout main` / `git checkout windows`.
- `source/`, `engine/`, CMakeLists.txt, README.md are cross-platform and shared — keep them in sync across branches; do not let the two copies diverge.
- `.vscode/tasks.json`, `.vscode/c_cpp_properties.json`, `.vscode/build.bat`, `.vscode/launch.json`, `.gitignore` are platform-specific (different per branch).
- AGENTS.md should stay identical on both branches to avoid merge conflicts.

## Architecture

- `source/main.cpp` — entry. `eng::CreatorEngine` is a singleton (`GetInstance()`), private default ctor, copy/move deleted. Registers a `Game*` (subclass of `eng::Application`) via `SetApplication`, then Init/Run/Destroy. `CreatorEngine::Init(w,h)` — args default to 0 = auto 80% of primary monitor, centered.
- `source/Game.{h,cpp}` — sample `eng::Application` subclass (Init/Update/Destroy).
- `engine/source/` — `eng` namespace (all lowercase):
  - `Application.{h,cpp}` — abstract base: `Init/Update(float deltaTime)/Destroy`, NeedsToBeClosed flag.
  - `CreatorEngine.{h,cpp}` — window creation (GLFW 3.3 core, GLEW init), game loop (deltaTime + `glfwPollEvents`/`SwapBuffers`), `keyCallback` routes to InputManager.
  - `input/InputManager.{h,cpp}` — `std::array<bool,256>` key states; friend of CreatorEngine (private ctor); exposes `SetKeyPressed`/`IsKeyPressed`.
  - `eng.h` — umbrella header including Application, CreatorEngine, InputManager.
- `CMakeLists.txt` + `engine/CMakeLists.txt` — cross-platform CMake build; Engine is a static lib linked by the app; include dirs exported PUBLIC.

## Adding a new source file

Must register it in **all three** places or builds break:
- `engine/CMakeLists.txt` (if in engine) or root `CMakeLists.txt` (if in source/)
- `.vscode/tasks.json` (macOS direct compile) — module list is explicit
- `.vscode/build.bat` (Windows)

## macOS (branch `main`)

```
/usr/bin/clang++ -g -I/opt/homebrew/include -Iengine/source -Isource \
  source/main.cpp source/Game.cpp engine/source/Application.cpp \
  engine/source/CreatorEngine.cpp engine/source/input/InputManager.cpp \
  -L/opt/homebrew/lib -lglfw -lGLEW -framework OpenGL -o main
```

- Must use `clang++`, not `clang` — linking with `clang` fails with missing C++ stdlib symbols.
- Homebrew lives at `/opt/homebrew` (Apple Silicon), never `/usr/local`.
- `-framework OpenGL` is required: the GLEW dylib does NOT export GL functions (e.g. `glClear`); they come from the system OpenGL framework.
- `-lglfw -lGLEW -framework OpenGL` all three needed together.
- CMake: `cmake -S . -B build && cmake --build build` (verified — finds glfw3/GLEW via Homebrew).
- `Cmd+Shift+B` = build then run in an **external Terminal window** via `open -a Terminal .vscode/run.command` (a gitignored script). Window auto-closes on clean exit.

### macOS gotchas

- **Renaming a dir to a different case fails** (`git mv Engine engine` → "Invalid argument") because APFS is case-insensitive. Rename via a temp name: `git mv Engine engine_tmp && git mv engine_tmp engine`.
- **Do NOT use osascript to drive Terminal.app** — it hangs waiting for macOS Automation permission. Use a `.command` script + `open -a Terminal` instead.
- **A compile error kills IntelliSense code completion** for the whole translation unit. Fix the error (e.g. wrong GLFW call signature), then "C/C++: Restart IntelliSense Server" if completion stays dead.

## Windows (branch `windows`)

- Build: `Ctrl+Shift+B` runs `.vscode/build.bat` — `vcvars64.bat` from `D:\VisualStudio2026` (VS 2026, MSVC 14.51), then `cl` with `/utf-8`, includes from `ThirdParty/include` + `source/` + `engine/source/`, links `glew32.lib glfw3dll.lib opengl32.lib`, copies DLLs next to the exe.
- Debug (F5): `.vscode/launch.json` → external console, `preLaunchTask` = build.bat.
- Deps in `ThirdParty/` (vcpkg `C:\vcpkg` artifacts: glfw3 3.4, glew 2.3.1). Refresh: re-copy from `C:\vcpkg\installed\x64-windows\{include,lib,bin}` into ThirdParty, then update build.bat (link) and c_cpp_properties.json (include).

### Windows gotchas

- Close the running `main.exe` before rebuilding — a live process locks the exe → LNK1168.
- `/utf-8` is mandatory: source has Chinese comments; without it cl emits C4819 under GBK code page 936.
- DLLs (`glew32.dll`, `glfw3.dll`) must sit next to `main.exe`; build.bat copies them.
- vcpkg downloads need local proxy `127.0.0.1:10090` (direct GitHub access fails). Only matters when refreshing `ThirdParty/`.
- **Do NOT delete the `C/C++: cl.exe 生成活动文件` cppbuild task** — the extension regenerates a broken one (no include/lib paths) if missing. Keep it; only the build.bat task should be `isDefault`.
- VS Code caches tasks.json: after editing, "Developer: Reload Window" before testing.

## IntelliSense

- IntelliSense reads `.vscode/c_cpp_properties.json` (platform-specific paths), not tasks.json. Stale entries in the Problems panel are common — rebuild or "C/C++: Reset IntelliSense Database" before assuming config is broken.