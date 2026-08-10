# CreatorEngine 创作者引擎

创作者引擎（CreatorEngine）是一个轻量级的跨平台游戏引擎项目，基于 C++ 开发，使用 GLFW 负责窗口创建与事件处理、GLEW 负责 OpenGL 扩展加载、GLM 负责数学运算，并内置基于 ImGui 的场景编辑器。

当前版本已实现：引擎窗口创建（默认屏幕 80%、居中）、OpenGL 3.3 核心模式上下文、增量时间游戏循环、场景（Scene）/ 游戏对象（GameObject）/ 组件（Component）框架、2D 精灵渲染、2D 物理、递归场景序列化，以及 Unity 风格 ImGui 编辑器。编辑器包含 Hierarchy、Scene/Game、Inspector、Project/Console，Scene View 可通过按钮在 2D 正交视图和 3D 透视视图之间切换。

脚本层采用 `Object -> Component -> Script -> CSharpScript` 原生继承链，并提供可选 Mono embedding 后端。启用 Mono 后，C# 脚本可接收 `Awake/Start/Update/OnDestroy` 生命周期并读写 GameObject 的 Transform 位置。

## 环境要求

- macOS（Apple Silicon 或 Intel）、Windows
- Homebrew（macOS 包管理器，Apple Silicon 路径为 `/opt/homebrew`）
- GLFW、GLEW、GLM（Windows 使用仓库内 `ThirdParty/` 预编译库，无需安装；ImGui、nlohmann/json、stb_image 已内置在 `ThirdParty/`）

macOS 下安装依赖：

```bash
brew install glfw glew glm
```

## 构建

项目支持 CMake 和 VS Code 任务两种构建方式。

### CMake（推荐）

```bash
cmake -S . -B build
cmake --build build
```

- macOS：自动通过 Homebrew 查找 GLFW/GLEW/GLM/OpenGL
- Windows：自动使用 `ThirdParty/` 中的库，并在构建后拷贝 DLL 到输出目录

### VS Code

- macOS：`Cmd+Shift+B` 运行 `tasks.json` 中的构建任务
- Windows：`Ctrl+Shift+B` 运行 `.vscode/build.bat`（MSVC + `ThirdParty/`）

### 常见编译坑（macOS）

- 必须使用 `clang++`，使用 `clang` 会因缺少 C++ 标准库而链接失败
- 必须链接 `-framework OpenGL`：macOS 上 GLEW 的动态库不导出任何 GL 函数（如 `glClear`）
- 三个链接参数缺一不可：`-lglfw -lGLEW -framework OpenGL`

## 运行

```bash
./main
```

窗口默认按主显示器 80% 尺寸创建并居中显示，也可在 `Init()` 中传入固定宽高。

## 编辑器工作流

- Hierarchy 支持任意深度父子结构、拖拽更改父级、创建子对象与递归删除。重挂父级时默认保持世界变换，并拒绝循环层级。
- Scene View 顶部的 `2D` / `3D` 是编辑器相机模式按钮。同一个 Scene 可以同时保存 2D 和 3D 对象，切换按钮不会改变场景文件类型或游戏相机。
- 2D Scene View 使用正交相机和 XY 网格；3D Scene View 使用透视相机和 XZ 无限网格。鼠标滚轮缩放，中键平移，3D 模式下右键拖动环绕视角。
- Scene 与 Game 使用独立的带深度附件帧缓冲。Game 标签始终使用场景中的 Camera 组件。
- Inspector 可编辑 Transform、Camera、SpriteRenderer、2D 刚体/碰撞体和 C# Script，并可添加或移除组件。
- 顶部播放控件决定是否执行 Scene 的物理和脚本更新。目前停止播放不会自动还原播放期间的场景改动。

场景采用版本化 JSON。Scene 统一拥有全部 GameObject，Transform 只保存父子关系；保存时从根对象递归写入，加载和删除也按完整子树处理。

## Mono C# 脚本

Mono 后端默认关闭，因此没有 Mono SDK 时原生编辑器仍可正常构建。Windows 的 `build.bat` 构建的是未启用 Mono 的版本；启用 Mono 请使用 CMake。

先生成托管 API 与示例游戏脚本：

```bash
dotnet build Assets/Scripts/CreatorGame.csproj --configuration Debug
```

安装 Mono embedding SDK 后配置原生运行时：

```bash
cmake -S . -B build -DCREATOR_ENABLE_MONO=ON
cmake --build build
```

Windows 如果 CMake 无法定位 SDK，请将 `MONO_ROOT` 设置为 Mono 安装目录，例如 `C:\Program Files\Mono`。CMake 会查找头文件、导入库和运行时 DLL，并把 DLL 复制到可执行文件旁。macOS 可通过 `pkg-config mono-2` 查找安装。

在 Inspector 中添加 `C# Script`，填写程序集路径、命名空间与类名。仓库中的示例为：

- Assembly: `../Assets/Scripts/bin/Debug/netstandard2.0/CreatorGame.dll`
- Namespace: `CreatorGame`
- Class: `VerticalBob`

托管基类位于 `managed/CreatorEngine.Managed/MonoBehaviour.cs`。当前桥接已支持生命周期与 Transform Position；程序集热重载、公开字段反射、资源 API、输入 API 和完整调试器仍是后续工作。

## 项目结构

```
├── source/                        # 应用层
│   ├── main.cpp                   # 入口：创建引擎并注册 Game 应用
│   ├── Game.cpp/.h                # 示例应用（继承 eng::Application）
│   └── shaders/                   # GLSL 着色器（sprite、grid2d、infinite_grid）
├── engine/source/                 # 引擎框架源码（eng 命名空间）
│   ├── CreatorEngine.cpp/.h       # 引擎核心：窗口创建、游戏循环、单例
│   ├── Application.cpp/.h         # 应用基类（虚接口）
│   ├── eng.h                      # 引擎统一头文件
│   ├── core/                      # ECS 框架：Object/Component/Transform/GameObject/Scene/Script
│   │   └── SceneSerializer        # 场景 JSON 序列化
│   ├── graphics/                  # ShaderProgram/Texture/Framebuffer/SpriteRenderer/SpriteBatch/Camera
│   ├── physics/                   # PhysicsWorld/Rigidbody2D/BoxCollider2D/CircleCollider2D
│   ├── scripting/                 # MonoRuntime/CSharpScript（Mono 后端可选）
│   ├── input/InputManager         # 键盘状态管理
│   └── editor/Editor              # ImGui 场景编辑器
├── managed/CreatorEngine.Managed/ # C# MonoBehaviour API 与原生内部调用声明
├── Assets/Scripts/                # 示例 C# 游戏脚本项目
├── tests/                         # 核心层级与序列化测试
├── CMakeLists.txt                 # 跨平台构建（根 + engine）
├── engine/CMakeLists.txt          # 引擎静态库构建
├── ThirdParty/                    # 第三方依赖（include/lib/bin + imgui/json/stb_image/glm）
├── .vscode/                       # 构建任务与调试配置
└── AGENTS.md                      # 面向 AI 助手的开发须知
```

## 里程碑

- [x] 引擎窗口创建（默认屏幕 80%、居中）
- [x] 创建 OpenGL 3.3 核心模式上下文
- [x] 初始化 GLEW
- [x] 引擎核心架构（CreatorEngine 单例 + Application 基类）
- [x] 增量时间游戏循环
- [x] 渲染管线（着色器、VAO/VBO）
- [x] 场景、实体组件系统（Scene/GameObject/Component/Transform）
- [x] 任意深度层级、拖拽重挂、递归删除与递归序列化
- [x] 输入系统（InputManager）
- [x] 2D 精灵渲染与相机
- [x] 2D 物理（刚体、碰撞盒）
- [x] 场景序列化（JSON）
- [x] Unity 风格 ImGui 编辑器工作区（Hierarchy/Scene/Game/Inspector/Project/Console）
- [x] Scene View 的 2D/3D 按钮切换与独立编辑器相机
- [x] 可选 Mono C# 生命周期与 Transform Position 桥接
- [ ] Mono 程序集热重载与 Inspector 字段反射
- [ ] 3D MeshRenderer、材质、灯光和 3D 物理
