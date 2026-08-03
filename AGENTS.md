# AGENTS.md

Single-file C++ OpenGL window project (macOS / Apple Silicon). No CMake, no tests, no git repo.

## Build

```
/usr/bin/clang++ -g -I/opt/homebrew/include -L/opt/homebrew/lib -lglfw -lGLEW -framework OpenGL main.cpp -o main
```

VS Code: Cmd+Shift+B runs the equivalent task from `.vscode/tasks.json`.

## Hard-won gotchas

- Must use `clang++`, not `clang` — linking with `clang` fails with missing C++ stdlib symbols.
- Homebrew lives at `/opt/homebrew` (Apple Silicon), never `/usr/local`.
- `-framework OpenGL` is required at link time: on macOS the GLEW dylib does NOT export GL functions (`glClear`, etc.); they come from the system OpenGL framework. Omitting it gives undefined-symbol linker errors.
- Keep `-lglfw -lGLEW` flags; all three are needed together.
- IntelliSense reads `.vscode/c_cpp_properties.json` (includePath `/opt/homebrew/include`), not `tasks.json`. "GLFW/glfw3.h file not found" errors from IntelliSense/cpptools may be stale entries in the Problems panel — rebuild before assuming config is broken.
- Adding a new dependency means updating both `tasks.json` (link flags) and `c_cpp_properties.json` (include paths) if the header lives outside `/opt/homebrew/include`.

## Structure

- `main.cpp` — the only source file; currently creates a GLFW window with a 3.3 core context, initializes GLEW, and runs a clear-color loop.
- `.vscode/tasks.json` — build task (single-file compile of the active file).
- `.vscode/c_cpp_properties.json` — IntelliSense config.
- Build outputs `main` and `main.dSYM` land in the repo root.
