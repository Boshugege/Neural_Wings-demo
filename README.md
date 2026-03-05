
```text
──────────────────────────────────────────────────────────────────────

  ███╗   ██╗ ███████╗ ██╗   ██╗ ██████╗   █████╗  ██╗     
  ████╗  ██║ ██╔════╝ ██║   ██║ ██╔══██╗ ██╔══██╗ ██║     
  ██╔██╗ ██║ █████╗   ██║   ██║ ██████╔╝ ███████║ ██║     
  ██║╚██╗██║ ██╔══╝   ██║   ██║ ██╔══██╗ ██╔══██║ ██║     
  ██║ ╚████║ ███████╗ ╚██████╔╝ ██║  ██║ ██║  ██║ ███████╗
  ╚═╝  ╚═══╝ ╚══════╝  ╚═════╝  ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚══════╝
░░▒▒▓▓███████████████████████████████████████████████▓▓▒▒░░
      __      __  ___   __    _   ______    _____
      \ \    / / |_ _| |  \  | | |  ____|  / ____|
       \ \  / /   | |  |   \ | | | |  __  | (___  
        \ \/ /    | |  | |\ \| | | | |_ |  \___ \ 
         \  /    _| |_ | | \   | | |__| |  ____) |
          \/    |_____||_|  \__|  \_____| |_____/ 
        
────────────────────────────────────────────────────────
 [ V1.0 ]
```
![alt text](https://img.shields.io/badge/Languages-C%2B%2B%20-blue?style=for-the-badge)
![alt text](https://img.shields.io/badge/Graphics-OpenGL%203.3-orange?style=for-the-badge&logo=opengl)
![alt text](https://img.shields.io/badge/Platform-Desktop%20%26%20Web-4fc08d?style=for-the-badge)
![alt text](https://img.shields.io/badge/UI-Vue%203-4fc08d?style=for-the-badge&logo=vue.js)

**Neural Wings** 是一款跨平台高性能空战竞技系统。它不仅是一个游戏示例，更是一套完整的 **3D 引擎解决方案**。通过一套核心代码，利用 WebAssembly 实现 Web 端“即点即玩”，同时通过原生编译为桌面端提供极致性能。

---

## 🛠️ 第一部分：Engine (通用引擎)

引擎层位于 `src/Engine`，是一套完全自研、高并发、且针对 WebAssembly 与原生桌面端双优化的 **3D 实时框架**。其设计核心在于“机制与策略分离”，确保了极高的通用性与工业级性能。

### 1. 核心架构与场景树 (Core & Scene Graph)
*   **极致优化的 ECS 架构**：不同于传统的单体类，采用游戏对象-组件系统。通过 **活跃对象索引 (Active Set)** 优化，系统仅遍历当前活动的实体，结合高性能 **对象池 (Object Pool)**，在大规模弹幕或碎片场景下将内存抖动降至接近零。
*   **层级变换系统 (Hierarchy)**：支持无限层级的父子节点嵌套。内置 **脏标记 (Dirty Flag)** 算法与递归矩阵更新逻辑。支持 **局部空间 (Local)** 与 **世界空间 (World)** 的动态切换，并在切换时实现坐标补偿以保持视觉位置一致。
*   **全局事件总线 (Event System)**：基于发布/订阅模式的“神经系统”。支持事件的延迟处理与自动注销，实现了物理碰撞、战斗逻辑、UI 反馈与特效触发之间的完全解耦。

### 2. 逻辑执行与脚本系统 (Logic & Scripting)
*   **C++ 脚本生命周期**：提供完善的 `OnCreate`, `OnWake`, `OnUpdate`, `OnFixedUpdate`, `OnSleep` 及 `OnDestroy` 挂钩。通过 **Scripting Factory** 动态实例化逻辑，支持在一个物体上挂载多个独立逻辑脚本。
*   **GameScreen 状态机**：基于工厂模式的屏幕管理器。管理从“开屏动画 -> 主菜单 -> 游戏世界 -> 设置”的完整跳转逻辑。支持各屏幕间的 **数据隔离与状态保存**，通过 `ApplySettings` 实现全局参数的一键同步。

### 3. 全局物理管线 (Global Physics)
*   **可插拔物理阶段 (Physics Stages)**：物理引擎由一系列引擎自带的阶段或者用户自己编写组成（如 `GravityStage`, `SolarStage`, `CollisionStage`）。开发者可通过 JSON 自由组合场景物理规则，甚至自定义流体物理。
*   **高保真刚体动力学**：支持基于 **惯性张量矩阵 (Inertia Tensors)** 的精确旋转解算、角动量积分与阻尼模拟，模拟出具有“重量感”的飞行体验。
*   **严格碰撞检测 (OBB SAT)**：定向包围盒（OBB）分离轴算法。不仅能检测碰撞，还能实时计算 **接触点 (Contact Points)**、**法线** 与 **穿透深度**，支持物理反弹与位置修正（Linear Projection）。
*   **高性能射线检测 (Raycasting)**：基于 **Slab Method** 的空间射线算法，利用活跃实体缓存实现即时命中判定（Hitscan），解决高速物体穿越难题。

### 4. 渲染管线与 Shader 系统 (Graphics & Shaders)
*   **Render Graph (渲染图系统)**：核心调度器管理一个 **命名渲染目标 (RT) 池**。通过 JSON 拓扑配置，支持非线性后处理链、多视口（Viewport）劫持渲染、以及多 RT 间的 **深度缓冲区共享 (Depth Sharing)**。
*   **原子级网格材质控制**：支持单一模型（Model）包含多个子网格（Mesh）。引擎允许通过 `meshPasses` 配置为**每一个独立的 Mesh 指定不同的 Shader 和渲染参数**（如战机的机身使用金属 Shader，座舱盖使用折射 Shader）。
*   **高级 Shader 包装器**：支持 Uniform 变量位置缓存（Location Caching）以消除字符串查询开销。支持 **多通行渲染 (Multi-pass)**，允许一个 Mesh 挂载多个 Shader 叠加效果（如基础材质 + 描边 + 护盾）。
*   **环境渲染**：支持 HDR 高动态范围全景图实时转 **Cubemap** 算法，内置 360 度天空盒系统。

### 5. 高性能 GPGPU 粒子模拟框架 (Particle System)
针对 WebGL 2.0 硬件限制进行深度的 GPGPU 优化，是引擎的视觉表现核心：
*   **Transform Feedback (TFB) 架构**：为了兼容不支持 Compute Shader 的 WebGL 2.0 平台，利用 TFB 技术将粒子物理模拟完全卸载至 GPU 顶点阶段。
*   **Ping-Pong 双缓冲机制**：通过两套 VBO 实现“乒乓交换”，计算结果直接在显存内部写回，实现零 CPU 拷贝的 10 万级粒子实时更新。
*   **PBO 纹理同步与邻居查询**：引入 **PBO (Pixel Buffer Object)** 机制，每帧将 VBO 数据极速同步至 **RGBA32F 数据纹理**。这使得 Shader 能够通过 `texelFetch` 实现高效的邻域查找，从而在 GPU 内部支持 **SPH (流体动力学)** 或 **类鸟群 (Boids)** 算法。
*   **双空间发射器 (Dual-Space Emitters)**：
    *   **随体系 (Local Space)**：粒子在局部系内模拟并随宿主变换。
    *   **世界系 (World Space)**：粒子出生即脱离宿主，适用于战机拉烟、爆炸残留等轨迹效果。
*   **实例化渲染 (Instanced Rendering)**：采用 **Instanced Billboard** 技术，单次 Draw Call 处理全场粒子，结合 **深度图采样 (Soft Particles)** 实现粒子与 3D 几何体的柔和交界。


### 6. 音频与交互系统 (Audio & Input)
*   **3D 空间音效系统**：基于相机基向量实时解算声源的 **距离衰减 (Attenuation)** 与 **左右方位感 (Panning)**。支持 **声音别名池 (Audio Aliasing)**，实现机炮等高频触发音效的完美重叠播放。
*   **抽象输入映射**：将硬件输入（键盘、鼠标、触摸）解耦为逻辑动作（Action）与轴（Axis）。支持数据驱动的键位绑定方案，内置 **输入拦截机制**（如聊天时拦截飞控输入）。

### 7. 数据驱动与资源复用 (Data & Resource)
*   **资源复用系统 (Resource Manager)**：实现模型、纹理、Shader、音频资源的全局单例化存储。支持 **GIF 序列自动解码** 并实时拼合为 Sprite Sheet，结合专用 Shader 提升动态贴图性能。
*   **预制件 (Prefab) 与场景加载**：支持递归解析 JSON 场景文件。预制件系统允许定义复杂的组件模板，并在 `SceneManager` 加载时实现组件参数的按需覆盖与相机的延迟挂载（Late-Binding）。

### 8. 混合 UI 框架 (Hybrid UI)
*   **Web-Tech 驱动的 UI**：本项目核心特色。使用 **Vue 3 + Vite** 构建所有 UI 界面，利用 Web 开发的极高效率与丰富的动画库（如 GSAP）实现极具冲击力的科幻 HUD。
*   **跨平台桥接层**：Web 端通过 Emscripten 导出接口，桌面端通过 **Ultralight** 嵌入高性能 WebView，实现了 C++ 引擎数据与 JavaScript UI 状态的同步响应。

### 9. 网络通信 (Networking)
*   **异构网络支持**：引擎层抽象了传输协议。Web 端采用基于 `libdatachannel` 的 **WebRTC Data Channel** 以突破浏览器 UDP 限制；桌面端采用原生 **ENet/UDP** 保证低延迟。
*   **状态同步机制**：支持跨平台实体状态同步、实时聊天消息路由及服务器权威校验接口。

---

## 🎮 第二部分：Neural Wings: The Demo (游戏层)

游戏层位于 `src/Game`，是引擎能力的具体应用，展示了多种先进的视觉效果与战斗逻辑。

### 1. 先进视觉效果示例 (Advanced Visuals)
基于引擎的 **Render Graph (渲染图)** 架构，游戏层实现了一系列现代渲染技术，证明了非线性管线处理复杂光影的能力：
*   **GTAO (地面真实环境光遮蔽)**：通过深度图采样与地平线搜索算法，为 Low-Poly 场景提供深邃的接触阴影与体积感。
*   **双边滤波去噪 (Bilateral Filtering)**：专门针对 GTAO 与流体深度图设计的去噪器，在模糊噪点的同时完美保留物体的几何边缘。
*   **体积云与宇宙尘埃**：利用全屏 Raymarching 技术，结合 3D 分形噪声，在无限空间中营造出具有厚度感与透光效果的星云氛围。
*   **Bloom 泛光系统**：提取高亮度像素并进行多级高斯模糊，为引擎喷口、导弹爆炸提供耀眼的视觉过载效果。

### 2. 高保真物理逻辑 (High-Fidelity Physics)
游戏层充分利用了引擎的 **OBB 物理核心** 与 **脚本系统**，构建了专业的航空动力学模型：
*   **6-DOF 飞行模型**：模拟真实的升力、阻力、推力与重力交互。支持攻角（AoA）计算、失速判定以及基于速度的机动灵敏度变化。
*   **导弹比例导航 (Proportional Navigation)**：不同于简单的“追尾”算法，游戏层实现了专业的比例导引逻辑。导弹能够根据目标的角速度预判拦截路径，配合 **Rigidbody 角动量积分**，呈现出极其丝滑且硬核的追踪轨迹。
*   **即时命中与受损反馈**：结合 **Raycasting (射线检测)** 实现机炮的即时判定，并通过事件系统触发蒙皮闪红、零件脱落与爆炸特效。

### 3. 流体模拟技术验证：SPH + SSF
这是本项目中最具代表性的技术案例，用于证明 **GPU 粒子管线** 与 **后处理系统** 的无缝联动：
*   **SPH (光滑粒子流体动力学)**：利用引擎的 **Transform Feedback** 架构与 **PBO 纹理同步** 机制，在 GPU 内部完成数千个流体粒子的压力、粘滞力与邻域搜索计算，实现实时流体物理。
*   **SSF (屏幕空间流体渲染)**：
    *   **深度/厚度提取**：将 SPH 粒子作为球体渲染到特定 RT，提取场景流体的深度与厚度信息。
    *   **平滑与重建**：通过后处理图中的 **双边滤波** 对流体深度进行平滑，消除颗粒感。
    *   **最终着色**：利用平滑后的深度图实时重建流体表面法线，结合场景贴图实现折射、反射与菲涅尔效应，将离散的粒子转化为连续的视觉水面。

### 4. 🎨 帧合成编辑器 (Frame Composition Editor)
为了简化复杂的后处理设计，项目内置了**可视化帧合成编辑器**：
*   **节点化工作流**：支持以节点拖动的方式制作渲染管线。
*   **可视化配置**：支持添加 **Pass 节点、粒子节点、贴图节点**，并直观连接各节点的 `inputs` 与 `outputs`。
*   **一键生成**：实时预览后处理逻辑，并可直接导出为引擎可读的 `view/renderview.json` 配置文件。
*   **导入支持**：支持导入已有的 `renderview.json` 文件，快速复现复杂的渲染效果。

![Frame Process Editor](./README/frameEditor.png)
---

## 📂 详细文件层级结构

```text
NeuralWings/
├── main.cpp                         # 全局引导与引擎初始化入口
├── shell.html                       # Emscripten WebGL 编译模版
│
├── Engine/                          # ================= 引擎层 (核心框架) =================
│   ├── Engine.h                     # 引擎单例与全局访问点
│   ├── Config/                      # 配置加载 (Config.cpp/h, EngineConfig.h)
│   ├── Core/                        # 核心逻辑
│   │   ├── GameWorld.cpp/h          # 场景容器与对象生命周期管理
│   │   ├── Components/              # ECS 组件 (Transform, RigidBody, Render, Script等)
│   │   ├── Events/                  # 事件总线 (EventManager, IEvent)
│   │   └── GameObject/              # 实体定义 (GameObject, Factory, Pool)
│   ├── Graphics/                    # 渲染基建
│   │   ├── Renderer.cpp/h           # 核心渲染器
│   │   ├── ShaderWrapper.cpp/h      # GLSL 包装器
│   │   ├── Camera/                  # 相机管理 (CameraManager, mCamera)
│   │   ├── Lighting/                # 灯光系统 (LightingManager)
│   │   ├── Particle/                # 粒子系统核心 (TFBManager, GPUBuffer, Emitter)
│   │   ├── PostProcess/             # 后处理调度 (PostProcesser, Pass)
│   │   ├── RenderView/              # 视口与视图管理 (RenderViewer)
│   │   └── Skybox/                  # 天空盒逻辑
│   ├── Math/                        # 数学库 (LinearAlgebra: Matrix/Vector, Complex, Calculus)
│   ├── Network/                     # 网络架构 (基于 nbnet)
│   │   ├── Chat/                    # 聊天管理
│   │   ├── Client/                  # 客户端标识与连接
│   │   ├── Protocol/                # 协议定义与序列化 (Messages.h)
│   │   ├── Sync/                    # 状态同步系统 (NetworkSyncSystem)
│   │   └── Transport/               # 传输层 (UDP/WebRTC/JS桥接)
│   ├── System/                      # 引擎子系统
│   │   ├── Audio/                   # OpenAL/WebAudio 包装 (AudioManager)
│   │   ├── HUD/                     # HUD 抽象层 (HudManager, HudBridgeScript)
│   │   ├── Input/                   # 抽象输入映射
│   │   ├── Physics/                 # 物理管线 (PhysicsSystem, Collision/Gravity Stages)
│   │   ├── Ray/                     # 射线检测算法 (mRay)
│   │   ├── Resource/                # 资源加载与生命周期 (ResourceManager)
│   │   ├── Scene/                   # 场景解析与切换
│   │   ├── Screen/                  # 屏幕/UI流控 (ScreenManager, ScreenFactory)
│   │   ├── Script/                  # 脚本驱动 (ScriptingSystem)
│   │   └── Time/                    # 定时器与时间步 (TimeManager, Timer)
│   ├── UI/                          # UI 渲染接口 (UltralightLayer, WebLayer)
│   └── Utils/                       # 通用工具 (JsonParser.h)
│
├── Game/                            # ================= 游戏层 (业务逻辑) =================
│   ├── Events/                      # 战斗事件 (CombatEvents.h)
│   ├── HUD/                         # 具体的 UI 元素 (姿态仪, 聊天框, 实体标签)
│   ├── Screen/                      # 具体的页面 (Start, MainMenu, Gameplay, Options)
│   ├── Scripts/                     # 游戏逻辑脚本 (飞控, 武器, 血量, 旋转器, 同步)
│   └── Systems/                     # 游戏专属物理/粒子算子
│       ├── Particles/               # 粒子初始化器 (SPH流体, 径向速度, 碰撞初始化)
│       └── Physics/                 # 自定义物理规则 (太阳系引力 SolarStage, 网络校验)
│
└── assets/                          # ================= 外部资产 (数据驱动) =================
    ├── config/                      # 外部 JSON 配置 (InputMap, AudioSettings)
    ├── prefabs/                     # 实体预制件 JSON (飞机、导弹属性)
    ├── scenes/                      # 关卡/场景定义文件
    ├── view/                        # 渲染管线图 (Render Graph) 拓扑配置
    ├── shaders/                     # GLSL 着色器库
    │   ├── lighting/                # 基础照明
    │   ├── outline/                 # 描边效果
    │   ├── particles/               # 粒子计算与渲染着色器
    │   ├── postprocess/             # 后处理 (体积云, 雾, AO)
    │   └── skybox/                  # 天空盒
    ├── textures/                    # 贴图与 GIF 序列
    ├── models/                      # 3D 模型 (.obj)
    └── sounds/                      # 音频资源 (WAV/MP3)
```

---

## 📦 资源文件 (Assets) 详解

### Shader 目录结构说明
*   **`shaders/particles/compute/`**: 存储 Transform Feedback 逻辑，负责计算粒子的运动行为（如 SPH 物理、引力计算）。
*   **`shaders/particles/render/`**: 粒子渲染 Shader，可直接输出数据到指定 RT（如 `fluid_depth.fs` 获取粒子深度，`fluid_thickness.fs` 累加厚度）。
*   **`shaders/lighting/`**: 光源系统专用的渲染着色器，处理多光源累加与阴影采样。
*   **`shaders/postprocess/`**: 后处理相关 Shader（如 GTAO、双边滤波、Bloom、体积云）。
*   **`shaders/skybox/`**: 天空盒渲染及全景图转换着色器。
*   **`shaders/texture/`**: 材质绑定与 UV 动画（GIF）处理着色器。
*   **`shaders/utils/`**: 引擎使用的功能性着色器（如深度可视化、调试线框）。
*   **其他着色器**: 用于绘制物体时使用的着色器（如 `rim_light.fs`, `hologram.fs`）。

---

## 许可证 (License)
本项目采用 MIT 许可证。部分底层窗口与音频接口参考了 Raylib 的跨平台实现思路。