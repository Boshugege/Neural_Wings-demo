# Neural Wings（神经之翼）

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/Platform-Web%20%7C%20Desktop-orange.svg" alt="Platform">
</p>

[English](./README_EN.md) | 中文

## 项目简介

**Neural Wings（神经之翼）** 是一个跨平台高性能空战竞技系统，采用 C++ 开发，通过 WebAssembly 技术实现网页端“即点即玩”，同时支持桌面端原生客户端提供媲美端游的极致体验。

本项目旨在打破端游与页游的界限，利用 C++ 的高性能特性与 WebAssembly 的跨平台能力，为用户提供低延迟、高帧率、物理拟真的 3D 空战服务。

## 核心特性

### 🚀 跨平台支持

- **Web 端**：基于 Emscripten 编译为 WebAssembly，运行于 Chrome/Edge 等现代浏览器
- **Desktop 端**：Windows 原生客户端，提供更极致的画质与性能
- **一套代码**：通过条件编译同时生成双平台客户端，数据互通

### 🎮 游戏功能

| 模块 | 功能描述 |
|------|----------|
| **飞行模拟** | 基于空气动力学的 6 自由度飞行控制（俯仰、滚转、偏航） |
| **战斗系统** | 机炮弹道计算、导弹锁定与追踪、物理碰撞检测 |
| **机库养成** | 战机改装（引擎、机翼、武器挂载），改装直接影响飞行物理参数 |
| **3D 预览** | 在机库中旋转查看战机的 Low Poly 模型 |
| **多人联机** | 房间管理、快速匹配、状态同步 |
| **社交系统** | 账号体系、排行榜、好友系统、实时聊天 |

### ⚡ 高性能架构

- **C++ Native First**：核心逻辑与渲染引擎采用 C++ 编写
- **GPU 硬件加速**：Raylib 引擎支持硬件加速渲染
- **低延迟网络**：
  - Web 端：WebRTC Data Channel（libdatachannel）实现毫秒级延迟
  - Desktop 端：原生 UDP（ENet/nbnet）
  - 异构网络架构：双端接入同一后端服务
- **GPU 粒子系统**：高效的粒子效果渲染

### 🛠️ 技术栈

| 模块 | 技术选型 |
|------|----------|
| **客户端核心** | Modern C++ (C++17), Raylib (渲染), RayGui (UI) |
| **Web 运行时** | Emscripten (WASM 编译器) |
| **网络通信 (Web)** | libdatachannel (基于 WebRTC) |
| **网络通信 (Desktop)** | ENet, nbnet (可靠 UDP) |
| **UI 框架** | Ultralight (桌面端 WebView), Vue 3 + Vite (Web 端) |
| **后端服务** | C++ (自定义游戏服务器框架) |
| **数据存储** | MySQL 8.0, Redis |
| **部署工具** | Docker & Docker Compose, Nginx, GitHub Actions |
| **构建系统** | CMake 3.11+ |

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                        客户端 Client                          │
├─────────────────────────┬───────────────────────────────────┤
│   Web 端 (WASM)         │   Desktop 端 (Native)            │
│   • Raylib 渲染          │   • Raylib + Ultralight         │
│   • WebRTC 通信          │   • 原生 UDP 通信               │
└───────────┬─────────────┴───────────────┬───────────────────┘
            │                             │
            ▼                             ▼
┌─────────────────────────────────────────────────────────────┐
│                      游戏网关 (C++)                          │
│              高并发服务器 · 异构网络协议支持                    │
└─────────────────────────────────────────────────────────────┘
            │                             │
            ▼                             ▼
┌──────────────────────┐    ┌────────────────────────────────┐
│    业务服务 (HTTP)    │    │      数据基础设施               │
│  • 用户账号管理       │    │  • Redis: 房间状态、Session   │
│  • 匹配逻辑           │    │  • MySQL: 用户数据、战绩历史   │
│  • 资产交易           │    └────────────────────────────────┘
└──────────────────────┘
```

## 构建指南

### 前提条件

- CMake 3.11+
- C++17 兼容编译器
- Windows: Visual Studio 2019+ / MinGW-w64
- Linux: GCC 9+ / Clang 10+
- Web: Emscripten SDK

### 桌面端构建

```bash
# 克隆仓库
git clone git@github.com:MeowLynxSea/Neural_Wings-demo.git
cd Neural_Wings-demo

# 创建构建目录
mkdir build && cd build

# 配置（Windows）
cmake .. -G "Visual Studio 17 2022" -A x64

# 配置（Linux）
cmake ..

# 构建
cmake --build . --config Release
```

### Web 端构建

```bash
# 设置 Emscripten 环境
source /path/to/emsdk/emsdk_env.sh

# 配置 Web 构建
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建
emmake cmake --build . --config Release
```

## 项目结构

```
Neural_Wings-demo/
├── src/
│   ├── Engine/                    # 引擎核心
│   │   ├── Config/               # 配置管理
│   │   ├── Core/                 # 核心组件（GameObject, Events, Components）
│   │   ├── Graphics/             # 图形渲染
│   │   │   ├── Camera/          # 摄像机管理
│   │   │   ├── Lighting/        # 光照系统
│   │   │   ├── Particle/         # GPU 粒子系统
│   │   │   ├── PostProcess/      # 后处理效果
│   │   │   ├── RenderView/       # 渲染视图
│   │   │   └── Skybox/           # 天空盒
│   │   ├── Math/                 # 数学库（矩阵、向量、复数）
│   │   ├── Network/              # 网络模块
│   │   │   ├── Chat/            # 聊天系统
│   │   │   ├── Client/          # 客户端
│   │   │   ├── Protocol/         # 协议定义
│   │   │   ├── Sync/             # 状态同步
│   │   │   └── Transport/        # 传输层
│   │   ├── System/               # 系统模块
│   │   │   ├── Audio/           # 音频系统
│   │   │   ├── HUD/              # 平视显示器
│   │   │   ├── Input/            # 输入处理
│   │   │   ├── Physics/          # 物理系统
│   │   │   ├── Scene/            # 场景管理
│   │   │   └── Script/           # 脚本系统
│   │   └── UI/                   # 用户界面
│   └── Game/                     # 游戏逻辑
│       ├── HUD/                  # 游戏 HUD
│       ├── Screen/              # 屏幕管理
│       ├── Scripts/              # 游戏脚本
│       └── Systems/             # 游戏系统
│
├── assets/                        # 资源文件
│   ├── models/                   # 3D 模型
│   ├── shaders/                  # 着色器
│   ├── textures/                 # 纹理
│   ├── sounds/                   # 音效
│   ├── scenes/                   # 场景文件
│   ├── prefabs/                  # 预制体
│   └── config/                   # 配置文件
│
├── ui/                           # Web 端 Vue 3 项目
├── frame-process-editor/          # 帧编辑工具 (TypeScript)
├── CMakeLists.txt                # 构建配置
└── 软件需求构思文档.md             # 需求规格说明
```

## 依赖项

项目通过 CMake FetchContent 自动下载以下依赖：

| 依赖 | 用途 |
|------|------|
| [raylib](https://www.raylib.com/) | 轻量级游戏开发库 |
| [raygui](https://github.com/raysan5/raygui) | 即时模式 GUI 框架 |
| [nlohmann-json](https://github.com/nlohmann/json) | JSON 解析库 |
| [nbnet](https://github.com/nathhB/nbnet) | 网络库 |
| [Ultralight](https://ultralig.ht/) | 轻量级 WebView（桌面端） |

## 潜在风险与对策

| 风险 | 对策 |
|------|------|
| C++ 内存管理复杂 | 利用 CMake 进行模块化管理 |
| WebRTC 集成难度大 | 优先跑通最小可行性 Demo（Echo Server） |
| WASM 调试困难 | 采用“桌面端开发，Web 端发布”策略 |

## 文档

- [软件需求构思文档](软件需求构思文档.md) - 完整的需求规格说明

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License

---

*本项目仅供学习交流使用。*
