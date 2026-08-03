# AGENTS.md

Single-file C++ OpenGL engine (GLFW + GLEW, OpenGL 3.3 core context). No CMake, no tests. One source file: `main.cpp`. Git repo with remote `origin` = https://github.com/ZLGame876/CreatorEngine.git.

## Dual-platform git workflow

- `main` branch = macOS config; `windows` branch = Windows config. Switch machines with `git checkout main` / `git checkout windows`.
- `main.cpp` is shared across branches (cross-platform code) — merge changes between branches; do not let the two copies diverge.
- `.vscode/tasks.json`, `.vscode/c_cpp_properties.json`, `.vscode/build.bat`, `.vscode/launch.json`, `.gitignore` are platform-specific (part of each branch).
- AGENTS.md should stay identical on both branches to avoid merge conflicts.

## Windows (branch `windows`, current machine)

- Build: `Ctrl+Shift+B` runs `.vscode/build.bat` — calls `vcvars64.bat` from `D:\VisualStudio2026` (VS 2026, MSVC 14.51), then `cl` with `/utf-8`, include path from `ThirdParty/include`, links `glew32.lib glfw3dll.lib opengl32.lib` from `ThirdParty/lib`, and copies DLLs next to the exe.
- Debug (F5): `.vscode/launch.json` → "CreatorEngine Debug" runs `Bin/main.exe` in an external console, `preLaunchTask` = build.bat task.
- Dependencies live in `ThirdParty/` (copied from vcpkg `C:\vcpkg` installed artifacts: glfw3 3.4, glew 2.3.1). To refresh: re-copy from `C:\vcpkg\installed\x64-windows\include` (include), `\lib\glew32.lib + glfw3dll.lib` (lib), `\bin\glew32.dll + glfw3.dll` (bin). New deps: `vcpkg install <pkg> --triplet x64-windows`, copy artifacts into ThirdParty, then add to build.bat (link) and c_cpp_properties.json (include).
- Windows SDK 10.0.26100.0; MSVC include/lib paths are hard-coded in `c_cpp_properties.json`.

### Windows gotchas

- **Close the running `main.exe` window before rebuilding** — a live process locks the exe and link fails with LNK1168.
- `/utf-8` is mandatory: source has Chinese comments; without it cl emits C4819 under GBK code page 936.
- DLLs (`glew32.dll`, `glfw3.dll`) must sit next to `main.exe`; build.bat copies them from `ThirdParty/bin`.
- vcpkg downloads require the local proxy `127.0.0.1:10090` (direct GitHub access fails); vcpkg auto-detects it via Windows IE proxy settings. Only needed when refreshing `ThirdParty/`.
- **Do NOT delete the `C/C++: cl.exe 生成活动文件` cppbuild task from tasks.json** — the C/C++ extension regenerates it if missing, and the regenerated version is broken (no include/lib paths). Keep it as-is (already patched); only the `build.bat` task should be `isDefault: true`.
- VS Code caches tasks.json: after editing it, run "Developer: Reload Window" before testing.

## macOS (branch `main`)

```
/usr/bin/clang++ -g -I/opt/homebrew/include -L/opt/homebrew/lib -lglfw -lGLEW -framework OpenGL main.cpp -o main
```

- Must use `clang++`, not `clang` — linking with `clang` fails with missing C++ stdlib symbols.
- Homebrew lives at `/opt/homebrew` (Apple Silicon), never `/usr/local`.
- `-framework OpenGL` is required: the GLEW dylib does NOT export GL functions; they come from the system OpenGL framework. Omitting it gives undefined-symbol linker errors.
- Keep `-lglfw -lGLEW`; all three flags are needed together.
- VS Code: Cmd+Shift+B runs the equivalent task from `.vscode/tasks.json`.

## IntelliSense

- IntelliSense reads `.vscode/c_cpp_properties.json` (platform-specific paths), not tasks.json. Stale entries in the Problems panel are common — rebuild or "C/C++: Reset IntelliSense Database" before assuming config is broken.

## Structure

- `source/main.cpp` — main entry; GLFW window (auto-sized to 80% of primary monitor, centered), 3.3 core context, GLEW init, shader compile/link, triangle render loop. Uses `Eng::CreatorEngine` framework via `source/Game.cpp` + `engine/source/eng.h`.
- `engine/` — engine framework source (`.h`/`.cpp` files being built out; CMakeLists.txt placeholder, no CMake build yet).
- `.vscode/build.bat` — Windows build script (Windows branch only); compiles `source/main.cpp` into `Bin/`.
- `.vscode/launch.json` — Windows F5 debug config (Windows branch only); runs `Bin/main.exe` in an external console.
- `.vscode/tasks.json`, `.vscode/c_cpp_properties.json` — platform-specific.
- `ThirdParty/` — vendored dependencies (include/, lib/, bin/), copied from vcpkg artifacts; refresh via vcpkg.
- `Bin/` — all build outputs (exe, dll, obj/pdb/ilk), gitignored. DLLs are copied here by build.bat.
