# CreatorEngine 创作者引擎

创作者引擎（CreatorEngine）是一个轻量级的跨平台游戏引擎项目，基于 C++ 开发，使用 GLFW 负责窗口创建与事件处理、GLEW 负责 OpenGL 扩展加载。

当前版本已实现：引擎窗口创建（默认 80% 屏幕尺寸、居中）、OpenGL 3.3 核心模式上下文、GLEW 初始化、基于 `Eng::Application` 的游戏循环（增量时间），以及三角渲染 Demo。

## 环境要求

- macOS（Apple Silicon 或 Intel）、Windows
- Homebrew（macOS 包管理器，Apple Silicon 路径为 `/opt/homebrew`）
- GLFW & GLEW（Windows 使用仓库内 `ThirdParty/` 预编译库，无需安装）

macOS 下安装依赖：

```bash
brew install glfw glew
```

## 构建

项目支持 CMake 和 VS Code 任务两种构建方式。

### CMake（推荐）

```bash
cmake -S . -B build
cmake --build build
```

- macOS：自动通过 Homebrew 查找 GLFW/GLEW/OpenGL
- Windows：自动使用 `ThirdParty/` 中的库，并在构建后拷贝 DLL 到输出目录

### VS Code

- macOS：`Cmd+Shift+B` 运行 `tasks.json` 中的构建任务
- Windows：`Ctrl+Shift+B` 运行 `.vscode/build.bat`（MSVC + `ThirdParty/`）

### macOS 直接编译

```bash
/usr/bin/clang++ -g -I/opt/homebrew/include -Iengine/source -Isource \
  source/main.cpp source/Game.cpp engine/source/Application.cpp engine/source/CreatorEngine.cpp \
  -L/opt/homebrew/lib -lglfw -lGLEW -framework OpenGL -o main
```

### 常见编译坑（macOS）

- 必须使用 `clang++`，使用 `clang` 会因缺少 C++ 标准库而链接失败
- 必须链接 `-framework OpenGL`：macOS 上 GLEW 的动态库不导出任何 GL 函数（如 `glClear`）
- 三个链接参数缺一不可：`-lglfw -lGLEW -framework OpenGL`

## 运行

```bash
./main
```

窗口默认按主显示器 80% 尺寸创建并居中显示，也可在 `Init()` 中传入固定宽高。

## 项目结构

```
├── source/main.cpp                # 入口：创建引擎并注册 Game 应用
├── source/Game.cpp/.h             # 示例应用（继承 Eng::Application）
├── engine/source/                 # 引擎框架源码
│   ├── CreatorEngine.cpp/.h       # 引擎核心：窗口创建、游戏循环
│   ├── Application.cpp/.h         # 应用基类（虚接口）
│   └── eng.h                      # 引擎统一头文件
├── CMakeLists.txt                 # 跨平台构建（根 + engine）
├── ThirdParty/                    # Windows 预编译依赖（include/lib/bin）
├── .vscode/                       # 构建任务与调试配置
└── AGENTS.md                      # 面向 AI 助手的开发须知
```

## 里程碑

- [x] 引擎窗口创建（默认屏幕 80%、居中）
- [x] 创建 OpenGL 3.3 核心模式上下文
- [x] 初始化 GLEW
- [x] 引擎核心架构（CreatorEngine + Application 基类）
- [x] 增量时间游戏循环
- [ ] 渲染管线（着色器、VAO/VBO）
- [ ] 场景、实体组件系统
- [ ] 输入系统
