# AGENTS.md

C++17 OpenGL game engine (GLFW + GLEW, OpenGL 3.3 core), 2D ECS + ImGui editor. Git repo, remote `origin` = https://github.com/ZLGame876/CreatorEngine.git. Two branches: `main` (macOS config) and `windows` (Windows config).

## Dual-platform git workflow

- `main` = macOS, `windows` = Windows; switch machines with `git checkout main` / `git checkout windows`.
- `source/`, `engine/`, `CMakeLists.txt`, `engine/CMakeLists.txt`, README.md, AGENTS.md, .gitignore are cross-platform and shared — keep them in sync across branches; do not let the two copies diverge.
- `.vscode/tasks.json`, `.vscode/c_cpp_properties.json`, `.vscode/build.bat`, `.vscode/launch.json`, `.vscode/run.command` are platform-specific (different per branch).
- ThirdParty/ is shared; glfw/glew/glm are needed on both platforms.

## Architecture

- `source/main.cpp` — entry. `eng::CreatorEngine` is a singleton (`GetInstance()`), private default ctor, copy/move deleted. Registers a `Game*` (subclass of `eng::Application`) via `SetApplication`, then Init/Run/Destroy. `CreatorEngine::Init(w,h)` — args default to 0 = auto 80% of primary monitor, centered.
- `source/Game.{h,cpp}` — sample `eng::Application` subclass (Init/Update/Destroy).
- `engine/source/` — `eng` namespace (all lowercase):
  - `CreatorEngine.{h,cpp}` — window creation (GLFW 3.3 core, GLEW init), game loop (deltaTime + `glfwPollEvents`/`SwapBuffers`), `keyCallback` routes to InputManager. Also sets working dir to exe dir at startup — shaders load relative to exe (Bin/), so `ShaderProgram::ReadFile` falls back to `../<path>` for the project-root `source/shaders/` copy.
  - `core/` — ECS: `Object`/`Component`/`Transform`/`GameObject`/`Scene`, `Script` (virtual Init/Update/OnDestroy), `SceneSerializer` (JSON, nlohmann/json).
  - `graphics/` — `shaderprogram`, `Texture` (stb_image), `SpriteRenderer`, `SpriteBatch`, `Camera`.
  - `physics/` — `PhysicsWorld`, `Rigidbody2D`, `Collider2D`, `BoxCollider2D`, `CircleCollider2D`, `PhysicsMaterial`.
  - `editor/` — ImGui-based editor: hierarchy panel, inspector, game view, scene save/load JSON. `Editor::Shutdown()` is idempotent (`m_IsShutdown` flag) — Game::Destroy and ~Editor both call it; do not remove the guard.
  - `input/InputManager.{h,cpp}` — `std::array<bool,256>` key states; friend of CreatorEngine (private ctor); exposes `SetKeyPressed`/`IsKeyPressed`.
  - `eng.h` — umbrella header including everything above (engine/Application.h, CreatorEngine.h, input, core, graphics, editor).
- `engine/CMakeLists.txt` — Engine static lib + ImGui sources; include dirs PUBLIC. Root `CMakeLists.txt` — app target, links Engine, third-party libs, copies DLLs POST_BUILD on Windows.
- Shaders: `source/shaders/*.{vert,frag}` (sprite, grid2d, infinite_grid).

## Adding a new source file

Must register it in **all three** places or builds break:
- `engine/CMakeLists.txt` (if in engine) or root `CMakeLists.txt` (if in source/) — both .cpp and .h lists
- `.vscode/build.bat` (Windows) — explicit module list
- `.vscode/tasks.json` (macOS direct compile) — module list is explicit

## Windows (branch `windows`)

- Build: `Ctrl+Shift+B` runs `.vscode/build.bat` — `vcvars64.bat` from `D:\VisualStudio2026` (VS 2026, MSVC 14.51), then `cl` with `/EHsc /std:c++17 /utf-8 /Zi`, includes `ThirdParty` + `ThirdParty/include` + `ThirdParty/imgui` + `ThirdParty/imgui/backends` + `source/` + `engine/source/`, links `glew32.lib glfw3dll.lib opengl32.lib`, outputs `Bin/main.exe` and copies `glew32.dll`/`glfw3.dll` next to it.
- Debug (F5): `.vscode/launch.json` → external console, `preLaunchTask` = build.bat.
- CMake also works on Windows: `cmake -S . -B build && cmake --build build` (uses same ThirdParty libs; run from repo root).
- CLion also works (`.idea/` + `cmake-build-debug/` are gitignored); any build config must stay in sync with build.bat module list.

### Windows gotchas

- Close the running `main.exe` before rebuilding — a live process locks the exe → LNK1168.
- `/utf-8` is mandatory: source has Chinese comments; without it cl emits C4819 under GBK code page 936.
- DLLs (`glew32.dll`, `glfw3.dll`) must sit next to `main.exe`; build.bat copies them.
- vcpkg downloads need local proxy `127.0.0.1:10090` (direct GitHub access fails). Only matters when refreshing `ThirdParty/`.
- **Do NOT delete the `C/C++: cl.exe 生成活动文件` cppbuild task** — the extension regenerates a broken one (no include/lib paths) if missing. Keep it; only the build.bat task should be `isDefault`.
- VS Code caches tasks.json: after editing, "Developer: Reload Window" before testing.

## macOS (branch `main`)

- Build: `Cmd+Shift+B` = build then run in an **external Terminal window** via `open -a Terminal .vscode/run.command` (a gitignored script). Window auto-closes on clean exit.
- Must use `clang++`, not `clang` — linking with `clang` fails with missing C++ stdlib symbols.
- Homebrew lives at `/opt/homebrew` (Apple Silicon), never `/usr/local`.
- `-framework OpenGL` is required: the GLEW dylib does NOT export GL functions (e.g. `glClear`); they come from the system OpenGL framework.
- `-lglfw -lGLEW -framework OpenGL` all three needed together. CMake finds glfw3/GLEW/glm via Homebrew.
- Deps via Homebrew: `brew install glfw glew glm` (glm 1.0.x header-only); nlohmann/json and stb_image are vendored in ThirdParty.

### macOS gotchas

- **Renaming a dir to a different case fails** (`git mv Engine engine` → "Invalid argument") because APFS is case-insensitive. Rename via a temp name: `git mv Engine engine_tmp && git mv engine_tmp engine`.
- **Do NOT use osascript to drive Terminal.app** — it hangs waiting for macOS Automation permission. Use a `.command` script + `open -a Terminal` instead.
- **A compile error kills IntelliSense code completion** for the whole translation unit. Fix the error, then "C/C++: Restart IntelliSense Server" if completion stays dead.

## ThirdParty (shared, both platforms)

- `ThirdParty/include` + `ThirdParty/lib` + `ThirdParty/bin`: glfw 3.4 + glew 2.3.1, copied from vcpkg `C:\vcpkg\installed\x64-windows\{include,lib,bin}` (Windows only; macOS uses Homebrew).
- `ThirdParty/include/glm`: GLM 1.0.3, header-only, cloned from `g-truc/glm@1.0.3` — vcpkg download of glm failed, manual clone worked.
- `ThirdParty/imgui/`: ImGui source compiled directly into the engine (docking branch), incl. backends `imgui_impl_glfw`/`imgui_impl_opengl3`.
- `ThirdParty/include/nlohmann/` (json), `ThirdParty/include/stb_image.h`: vendored headers, no extra setup.

## IntelliSense

- IntelliSense reads `.vscode/c_cpp_properties.json` (platform-specific paths), not tasks.json. Stale entries in the Problems panel are common — rebuild or "C/C++: Reset IntelliSense Database" before assuming config is broken.
