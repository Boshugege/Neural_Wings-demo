```text
─────────────────────────────────────────────────────────────────────────────────

  ███╗   ██╗███████╗██╗   ██╗██████╗  █████╗  ██╗     
  ████╗  ██║██╔════╝██║   ██║██╔══██╗██╔══██╗ ██║     
  ██╔██╗ ██║█████╗  ██║   ██║██████╔╝███████║ ██║     
  ██║╚██╗██║██╔══╝  ██║   ██║██╔══██╗██╔══██║ ██║     
  ██║ ╚████║███████╗╚██████╔╝██║  ██║██║  ██║ ███████╗
  ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚══════╝
    ░░▒▒▓▓████████████████████████████████▓▓▒▒░░
    __      __  ___   __    _   ______    _____
    \ \    / / |_ _| |  \  | | |  ____|  / ____|
     \ \  / /   | |  |   \ | | | |  __  | (___  
      \ \/ /    | |  | |\ \| | | | |_ |  \___ \ 
       \  /    _| |_ | | \   | | |__| |  ____) |
        \/    |_____||_|  \__|  \_____| |_____/ 
        
─────────────────────────────────────────────────────────────────────────────────
 [ V1.0 ]
```
![alt text](https://img.shields.io/badge/Languages-C%2B%2B%20-blue?style=for-the-badge)
![alt text](https://img.shields.io/badge/Graphics-OpenGL%203.3-orange?style=for-the-badge&logo=opengl)
![alt text](https://img.shields.io/badge/Platform-Desktop%20%26%20Web-4fc08d?style=for-the-badge)
![alt text](https://img.shields.io/badge/UI-Vue%203-4fc08d?style=for-the-badge&logo=vue.js)

**Neural Wings** is a cross-platform, high-performance air combat competitive system. More than just a game demo, it is a complete **3D engine solution**. Utilizing a single core codebase, it achieves "click-and-play" accessibility via WebAssembly for the web, while providing peak performance through native compilation for desktop.

---

## 🛠️ Part I: Engine (Core Framework)

Located in `src/Engine`, the engine layer is a self-developed, high-concurrency **3D real-time framework** optimized for both WebAssembly and native desktop. Its core design philosophy is "Separation of Mechanism and Policy," ensuring industrial-grade performance and high versatility.

### 1. Core Architecture & Scene Graph
*   **Highly Optimized ECS Architecture**: Unlike traditional monolithic classes, it utilizes a GameObject-Component system. Optimized via **Active Set indexing**, the system only traverses active entities. Combined with a high-performance **Object Pool**, memory jitter is reduced to near-zero during massive projectile or debris-heavy scenes.
*   **Hierarchical Transformation System**: Supports infinite nesting of parent-child nodes. Features a built-in **Dirty Flag** algorithm with recursive matrix updates. Supports dynamic switching between **Local** and **World** space with coordinate compensation to maintain visual consistency.
*   **Global Event Bus**: A Pub/Sub-based "nervous system." Supports deferred event processing and auto-unregistration, enabling complete decoupling between physical collisions, combat logic, UI feedback, and VFX triggers.

### 2. Logic Execution & Scripting
*   **C++ Script Lifecycle**: Provides robust hooks for `OnCreate`, `OnWake`, `OnUpdate`, `OnFixedUpdate`, `OnSleep`, and `OnDestroy`. Logic is dynamically instantiated via a **Scripting Factory**, allowing multiple independent scripts to be attached to a single object.
*   **GameScreen State Machine**: A factory-pattern-based screen manager. It handles the complete flow from "Splash Screen -> Main Menu -> Game World -> Settings." Supports **data isolation and state persistence** between screens, with `ApplySettings` enabling one-click global parameter synchronization.

### 3. Global Physics Pipeline
*   **Pluggable Physics Stages**: The physics engine consists of a sequence of built-in or user-defined stages (e.g., `GravityStage`, `SolarStage`, `CollisionStage`). Developers can configure scene physics rules via JSON or implement custom fluid physics.
*   **High-Fidelity Rigid Body Dynamics**: Supports precise rotation calculation, angular momentum integration, and damping simulation based on **Inertia Tensors**, delivering a "weighty" flight experience.
*   **Strict Collision Detection (OBB SAT)**: Oriented Bounding Box (OBB) Separation Axis Theorem algorithm. Not only detects collisions but also calculates **Contact Points**, **Normals**, and **Penetration Depth** in real-time, supporting physical bounce and Linear Projection correction.
*   **High-Performance Raycasting**: A spatial ray algorithm based on the **Slab Method**. It utilizes active entity caching for instant Hitscan determination, solving the "tunneling" issue for high-speed objects.

### 4. Rendering Pipeline & Shader System
*   **Render Graph System**: A core scheduler managing a pool of **Named Render Targets (RT)**. Through JSON topology configuration, it supports non-linear post-processing chains, viewport hijacking, and **Depth Buffer Sharing** between multiple RTs.
*   **Atomic Mesh & Material Control**: A single Model can contain multiple Sub-meshes. The engine allows **different Shaders and rendering parameters for each independent Mesh** via `meshPasses` configuration (e.g., metallic shader for the fuselage, refraction shader for the cockpit canopy).
*   **Advanced Shader Wrapper**: Features Uniform location caching to eliminate string-query overhead. Supports **Multi-pass rendering**, allowing a Mesh to stack multiple shaders (e.g., Base Material + Outline + Shield).
*   **Environment Rendering**: Supports real-time HDR equirectangular-to-**Cubemap** conversion algorithms and a built-in 360-degree Skybox system.

### 5. High-Performance GPGPU Particle Framework
Deep GPGPU optimization tailored for WebGL 2.0 hardware constraints forms the core of the engine's visual fidelity:
*   **Transform Feedback (TFB) Architecture**: To bypass the lack of Compute Shaders on WebGL 2.0, TFB technology offloads particle physics simulation entirely to the GPU vertex stage.
*   **Ping-Pong Double Buffering**: Uses two sets of VBOs for "Ping-Pong" swapping. Calculation results are written back directly within VRAM, achieving real-time updates for 100k+ particles with zero CPU copying.
*   **PBO Texture Synchronization & Neighborhood Query**: Introduces **PBO (Pixel Buffer Object)** mechanisms to sync VBO data per frame into **RGBA32F Data Textures**. This allows Shaders to use `texelFetch` for efficient neighborhood searches, enabling **SPH (Fluid Dynamics)** or **Boids** algorithms entirely on the GPU.
*   **Dual-Space Emitters**:
    *   **Local Space**: Particles simulate within a local frame and transform with the host.
    *   **World Space**: Particles detach from the host upon birth, ideal for jet trails and explosion remnants.
*   **Instanced Rendering**: Utilizes **Instanced Billboard** technology to process all particles in a single Draw Call. Combined with **Soft Particles** (depth map sampling), it ensures smooth intersections between particles and 3D geometry.

### 6. Audio & Interaction Systems
*   **3D Spatial Audio System**: Real-time calculation of **Attenuation** and **Panning** based on camera basis vectors. Supports **Audio Aliasing Pools**, allowing high-frequency sound effects like cannons to overlap perfectly.
*   **Abstract Input Mapping**: Decouples hardware input (Keyboard, Mouse, Touch) into logical **Actions** and **Axes**. Supports data-driven keybinding and **Input Interception** (e.g., blocking flight controls while typing in chat).

### 7. Data-Driven Workflow & Resource Reuse
*   **Resource Manager**: Implements global singleton storage for models, textures, shaders, and audio. Supports **automatic GIF sequence decoding** and real-time atlas generation into Sprite Sheets, boosting performance for dynamic textures via specialized shaders.
*   **Prefabs & Scene Loading**: Supports recursive parsing of JSON scene files. The Prefab system allows for complex component templates with on-demand parameter overrides and Late-Binding for camera attachments during `SceneManager` loading.

### 8. Hybrid UI Framework
*   **Web-Tech Driven UI**: A core feature of the project. Uses **Vue 3 + Vite** to build all UI interfaces, leveraging modern web development efficiency and animation libraries (like GSAP) to create high-impact sci-fi HUDs.
*   **Cross-Platform Bridge**: The web version exports interfaces via Emscripten, while the desktop version embeds a high-performance WebView via **Ultralight**, enabling synchronized state between the C++ engine and JavaScript UI.

### 9. Networking
*   **Heterogeneous Network Support**: The engine abstracts the transport protocol. The Web uses **WebRTC Data Channels** (via `libdatachannel`) to bypass browser UDP limits; Desktop uses native **ENet/UDP** for low latency.
*   **State Synchronization**: Supports cross-platform entity state sync, real-time chat routing, and server-authoritative validation interfaces.

---

## 🎮 Part II: Neural Wings: The Demo (Game Layer)

Located in `src/Game`, the game layer is the concrete application of the engine's capabilities, showcasing advanced visuals and combat logic.

### 1. Advanced Visuals
Based on the **Render Graph** architecture, the game implements several modern rendering techniques:
*   **GTAO (Ground Truth Ambient Occlusion)**: Uses depth map sampling and horizon search to provide deep contact shadows and a sense of volume in Low-Poly scenes.
*   **Bilateral Filtering**: A custom denoiser designed for GTAO and fluid depth maps, blurring noise while perfectly preserving geometric edges.
*   **Volumetric Clouds & Cosmic Dust**: Uses full-screen Raymarching and 3D fractal noise to create nebulae with thickness and light-scattering effects in infinite space.
*   **Bloom System**: Extracts high-luminance pixels and applies multi-level Gaussian blurs to provide dazzling visual overload for engine nozzles and missile explosions.

### 2. High-Fidelity Physics Logic
Utilizing the **OBB Physics Core** and **Scripting System**, the game implements a professional aero-ballistics model:
*   **6-DOF Flight Model**: Simulates real interactions between Lift, Drag, Thrust, and Gravity. Supports Angle of Attack (AoA) calculation, stall detection, and velocity-based maneuverability scaling.
*   **Proportional Navigation (Missiles)**: Unlike simple "tail-chase" algorithms, the game implements professional proportional guidance. Missiles predict interception paths based on the target's angular velocity, resulting in incredibly smooth and hardcore tracking trajectories.
*   **Hitscan & Damage Feedback**: Combines **Raycasting** for instant cannon hit detection with event-triggered effects like hull flashing, part detachment, and explosion VFX.

### 3. Fluid Simulation Tech: SPH + SSF
A signature technical demonstration of the **GPU Particle Pipeline** and **Post-Processing** synergy:
*   **SPH (Smoothed Particle Hydrodynamics)**: Utilizes the engine's **Transform Feedback** and **PBO Sync** to calculate pressure, viscosity, and neighborhood searches for thousands of fluid particles entirely on the GPU.
*   **SSF (Screen Space Fluid Rendering)**:
    *   **Depth/Thickness Extraction**: Renders SPH particles as spheres to specific RTs to extract depth and thickness.
    *   **Smoothing & Reconstruction**: Uses **Bilateral Filtering** within the Render Graph to smooth fluid depth and eliminate graininess.
    *   **Final Shading**: Reconstructs surface normals from smoothed depth, combining scene textures for refraction, reflection, and Fresnel effects, transforming discrete particles into a continuous liquid surface.

---

## 📂 Detailed File Structure

```text
NeuralWings/
├── main.cpp                         # Global entry point & engine initialization
├── shell.html                       # Emscripten WebGL compilation template
│
├── Engine/                          # ================= Engine Layer (Core Framework) =================
│   ├── Engine.h                     # Engine singleton & global access
│   ├── Config/                      # Config loading (Config.cpp/h, EngineConfig.h)
│   ├── Core/                        # Core logic
│   │   ├── GameWorld.cpp/h          # Scene container & object lifecycle
│   │   ├── Components/              # ECS Components (Transform, RigidBody, Render, Script, etc.)
│   │   ├── Events/                  # Event Bus (EventManager, IEvent)
│   │   └── GameObject/              # Entity definitions (GameObject, Factory, Pool)
│   ├── Graphics/                    # Rendering infrastructure
│   │   ├── Renderer.cpp/h           # Core Renderer
│   │   ├── ShaderWrapper.cpp/h      # GLSL Wrapper
│   │   ├── Camera/                  # Camera management (CameraManager)
│   │   ├── Lighting/                # Lighting system (LightingManager)
│   │   ├── Particle/                # Particle core (TFBManager, GPUBuffer, Emitter)
│   │   ├── PostProcess/             # Post-process scheduling (PostProcesser, Pass)
│   │   ├── RenderView/              # Viewport & View management
│   │   └── Skybox/                  # Skybox logic
│   ├── Math/                        # Math library (LinearAlgebra, Complex, Calculus)
│   ├── Network/                     # Network architecture (based on nbnet)
│   │   ├── Chat/                    # Chat management
│   │   ├── Client/                  # Client ID & Connection
│   │   ├── Protocol/                # Definitions & Serialization (Messages.h)
│   │   ├── Sync/                    # State synchronization (NetworkSyncSystem)
│   │   └── Transport/               # Transport layer (UDP/WebRTC/JS Bridge)
│   ├── System/                      # Engine subsystems
│   │   ├── Audio/                   # OpenAL/WebAudio Wrapper (AudioManager)
│   │   ├── HUD/                     # HUD abstraction (HudManager, HudBridgeScript)
│   │   ├── Input/                   # Abstract Input Mapping
│   │   ├── Physics/                 # Physics Pipeline (PhysicsSystem, Collision/Gravity Stages)
│   │   ├── Ray/                     # Raycasting algorithms (mRay)
│   │   ├── Resource/                # Resource loading & lifecycle (ResourceManager)
│   │   ├── Scene/                   # Scene parsing & switching
│   │   ├── Screen/                  # Screen/UI flow control (ScreenManager, ScreenFactory)
│   │   ├── Script/                  # Scripting drive (ScriptingSystem)
│   │   └── Time/                    # Timers & Timestep (TimeManager, Timer)
│   ├── UI/                          # UI rendering interfaces (UltralightLayer, WebLayer)
│   └── Utils/                       # General utilities (JsonParser.h)
│
├── Game/                            # ================= Game Layer (Business Logic) =================
│   ├── Events/                      # Combat events (CombatEvents.h)
│   ├── HUD/                         # Specific UI elements (Attitude indicator, Chat, Labels)
│   ├── Screen/                      # Specific pages (Start, MainMenu, Gameplay, Options)
│   ├── Scripts/                     # Game logic scripts (Flight, Weapons, HP, Rotor, Sync)
│   └── Systems/                     # Game-specific physics/particle operators
│       ├── Particles/               # Particle initializers (SPH, Radial velocity, Collision init)
│       └── Physics/                 # Custom physics rules (Solar gravity, Network validation)
│
└── assets/                          # ================= External Assets (Data-Driven) =================
    ├── config/                      # External JSON configs (InputMap, AudioSettings)
    ├── prefabs/                     # Entity prefab JSONs (Aircraft, Missile properties)
    ├── scenes/                      # Level/Scene definition files
    ├── view/                        # Render Graph topology configurations
    ├── shaders/                     # GLSL shader library
    │   ├── lighting/                # Base lighting
    │   ├── outline/                 # Outline effects
    │   ├── particles/               # Particle compute & render shaders
    │   ├── postprocess/             # Post-processing (Volumetric, Fog, AO)
    │   └── skybox/                  # Skybox shaders
    ├── textures/                    # Textures & GIF sequences
    ├── models/                      # 3D Models (.obj)
    └── sounds/                      # Audio resources (WAV/MP3)
```

---

## 📦 Assets Detail

### Shader Directory Structure
*   **`shaders/particles/compute/`**: Stores Transform Feedback logic for calculating particle behavior (e.g., SPH physics, gravity).
*   **`shaders/particles/render/`**: Particle rendering shaders (e.g., `fluid_depth.fs` for depth extraction, `fluid_thickness.fs` for accumulation).
*   **`shaders/lighting/`**: Specialized lighting shaders handling multi-light accumulation and shadow sampling.
*   **`shaders/postprocess/`**: Post-processing effects (GTAO, Bilateral Filtering, Bloom, Volumetric Clouds).
*   **`shaders/skybox/`**: Skybox rendering and equirectangular-to-cubemap conversion.
*   **`shaders/texture/`**: Material binding and UV animation (GIF) handling.
*   **`shaders/utils/`**: Functional shaders (Depth visualization, Debug wireframes).
*   **Other shaders**: Object-specific rendering (e.g., `rim_light.fs`, `hologram.fs`).

---

## License
This project is licensed under the MIT License. Portions of the low-level window and audio interfaces reference the cross-platform design philosophy of Raylib.