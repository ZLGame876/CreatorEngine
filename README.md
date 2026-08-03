# CreatorEngine 创作者引擎

创作者引擎（CreatorEngine）是一个轻量级的跨平台游戏引擎项目，基于 C++ 开发，使用 GLFW 负责窗口创建与事件处理、GLEW 负责 OpenGL 扩展加载。

当前版本已实现新建窗口、创建 3.3 核心模式的 OpenGL 上下文、初始化 GLEW，以及循环清屏渲染的基础框架。

## 环境要求

- macOS（Apple Silicon 或 Intel）、Linux、Windows
- Homebrew（macOS 包管理器，Apple Silicon 路径为 `/opt/homebrew`）
- GLFW & GLEW

macOS 下安装依赖：

```bash
brew install glfw glew
```

## 构建

项目采用单文件构建，无需 CMake。macOS / Apple Silicon 下使用系统 clang++ 直接编译：

```bash
/usr/bin/clang++ -g -I/opt/homebrew/include -L/opt/homebrew/lib \
  -lglfw -lGLEW -framework OpenGL main.cpp -o main
```

VS Code 用户：按 `Cmd+Shift+B` 即可运行 `tasks.json` 中配置好的构建任务。

### 常见编译坑

- 必须使用 `clang++`，使用 `clang` 会因缺少 C++ 标准库而链接失败
- 必须链接 `-framework OpenGL`：macOS 上 GLEW 的动态库不导出任何 GL 函数（如 `glClear`）
- 三个链接参数缺一不可：`-lglfw -lGLEW -framework OpenGL`

## 运行

```bash
./main
```

## 项目结构

```
├── main.cpp                       # 唯一源文件：窗口创建、GLEW 初始化、清屏渲染循环
├── .vscode/tasks.json             # VS Code 构建任务（编译命令）
├── .vscode/c_cpp_properties.json  # IntelliSense 头文件路径配置
└── AGENTS.md                      # 面向 AI 助手的开发须知
```

## 里程碑

- [x] 创建 GLFW 窗口（1280×720）
- [x] 创建 OpenGL 3.3 核心模式上下文
- [x] 初始化 GLEW
- [ ] 渲染管线（着色器、VAO/VBO）
- [ ] 引擎核心架构（场景、实体组件系统）
- [ ] 输入系统