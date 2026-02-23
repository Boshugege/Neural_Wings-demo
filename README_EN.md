# Neural Wings

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/License-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/Platform-Web%20%7C%20Desktop-orange.svg" alt="Platform">
</p>

English | [中文](./README.md)

## Overview

**Neural Wings** is a cross-platform high-performance air combat game built with C++. It leverages WebAssembly technology to enable "click-to-play" in web browsers while also supporting native desktop clients for an immersive gaming experience comparable to standalone games.

This project aims to bridge the gap between desktop and web gaming by utilizing C++'s high-performance capabilities combined with WebAssembly's cross-platform capabilities, delivering low-latency, high-frame-rate, physics-based 3D air combat.

## Key Features

### 🚀 Cross-Platform Support

- **Web**: Compiled to WebAssembly via Emscripten, runs in modern browsers (Chrome/Edge)
- **Desktop**: Native Windows client for ultimate graphics and performance
- **Single Codebase**: Generates both platforms via conditional compilation with shared data

### 🎮 Game Features

| Module | Description |
|--------|-------------|
| **Flight Simulation** | Aerodynamics-based 6-DOF flight control (pitch, roll, yaw) |
| **Combat System** | Ballistics calculation, missile locking & tracking, collision detection |
| **Hangar System** | Aircraft modification (engines, wings, weapons) - upgrades affect flight physics |
| **3D Preview** | Rotate and view aircraft Low Poly models in hangar |
| **Multiplayer** | Room management, quick match, state synchronization |
| **Social** | Account system, leaderboards, friends, real-time chat |

### ⚡ High-Performance Architecture

- **C++ Native First**: Core logic and rendering engine written in C++
- **GPU Acceleration**: Raylib engine with hardware-accelerated rendering
- **Low-Latency Networking**:
  - Web: WebRTC Data Channel (libdatachannel) for millisecond-level latency
  - Desktop: Native UDP (ENet/nbnet)
  - Heterogeneous network architecture: both clients connect to the same server
- **GPU Particle System**: Efficient particle effect rendering

### 🛠️ Tech Stack

| Module | Technology |
|--------|------------|
| **Client Core** | Modern C++ (C++17), Raylib (Rendering), RayGui (UI) |
| **Web Runtime** | Emscripten (WASM Compiler) |
| **Networking (Web)** | libdatachannel (WebRTC-based) |
| **Networking (Desktop)** | ENet, nbnet (Reliable UDP) |
| **UI Framework** | Ultralight (Desktop WebView), Vue 3 + Vite (Web) |
| **Backend** | C++ (Custom Game Server Framework) |
| **Database** | MySQL 8.0, Redis |
| **Deployment** | Docker & Docker Compose, Nginx, GitHub Actions |
| **Build System** | CMake 3.11+ |

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         Client                               │
├─────────────────────────┬───────────────────────────────────┤
│   Web (WASM)            │   Desktop (Native)                │
│   • Raylib Rendering   │   • Raylib + Ultralight           │
│   • WebRTC Communication│   • Native UDP Communication     │
└───────────┬─────────────┴───────────────┬───────────────────┘
            │                             │
            ▼                             ▼
┌─────────────────────────────────────────────────────────────┐
│                   Game Gateway (C++)                       │
│            High Concurrency Server ·                        │
│            Heterogeneous Network Protocol Support           │
└─────────────────────────────────────────────────────────────┘
            │                             │
            ▼                             ▼
┌──────────────────────┐    ┌────────────────────────────────┐
│  Business Services   │    │     Data Infrastructure        │
│  • User Account      │    │  • Redis: Room State, Session  │
│  • Matchmaking       │    │  • MySQL: User Data, History   │
│  • Asset Management  │    └────────────────────────────────┘
└──────────────────────┘
```

## Build Guide

### Prerequisites

- CMake 3.11+
- C++17 compatible compiler
- Windows: Visual Studio 2019+ / MinGW-w64
- Linux: GCC 9+ / Clang 10+
- Web: Emscripten SDK

### Desktop Build

```bash
# Clone repository
git clone git@github.com:MeowLynxSea/Neural_Wings-demo.git
cd Neural_Wings-demo

# Create build directory
mkdir build && cd build

# Configure (Windows)
cmake .. -G "Visual Studio 17 2022" -A x64

# Configure (Linux)
cmake ..

# Build
cmake --build . --config Release
```

### Web Build

```bash
# Set up Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Configure for Web
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
emmake cmake --build . --config Release
```

## Project Structure

```
Neural_Wings-demo/
├── src/
│   ├── Engine/                    # Engine core
│   │   ├── Config/               # Configuration management
│   │   ├── Core/                 # Core components (GameObject, Events, Components)
│   │   ├── Graphics/             # Graphics rendering
│   │   │   ├── Camera/          # Camera management
│   │   │   ├── Lighting/        # Lighting system
│   │   │   ├── Particle/        # GPU particle system
│   │   │   ├── PostProcess/     # Post-processing effects
│   │   │   ├── RenderView/      # Render view
│   │   │   └── Skybox/          # Skybox
│   │   ├── Math/                 # Math library (Matrix, Vector, Complex)
│   │   ├── Network/              # Network module
│   │   │   ├── Chat/            # Chat system
│   │   │   ├── Client/          # Client
│   │   │   ├── Protocol/        # Protocol definitions
│   │   │   ├── Sync/            # State synchronization
│   │   │   └── Transport/       # Transport layer
│   │   ├── System/               # System module
│   │   │   ├── Audio/           # Audio system
│   │   │   ├── HUD/             # Heads-up display
│   │   │   ├── Input/           # Input handling
│   │   │   ├── Physics/         # Physics system
│   │   │   ├── Scene/           # Scene management
│   │   │   └── Script/          # Script system
│   │   └── UI/                   # User interface
│   └── Game/                     # Game logic
│       ├── HUD/                  # Game HUD
│       ├── Screen/               # Screen management
│       ├── Scripts/              # Game scripts
│       └── Systems/              # Game systems
│
├── assets/                        # Resource files
│   ├── models/                   # 3D models
│   ├── shaders/                  # Shaders
│   ├── textures/                 # Textures
│   ├── sounds/                   # Sound effects
│   ├── scenes/                   # Scene files
│   ├── prefabs/                  # Prefabs
│   └── config/                   # Configuration files
│
├── ui/                           # Web Vue 3 project
├── frame-process-editor          # Frame editor tool (TypeScript)
├── CMakeLists.txt                # Build configuration
└── 软件需求构思文档.md             # Requirements specification (Chinese)
```

## Dependencies

The project automatically downloads the following dependencies via CMake FetchContent:

| Dependency | Purpose |
|------------|---------|
| [raylib](https://www.raylib.com/) | Lightweight game development library |
| [raygui](https://github.com/raysan5/raygui) | Immediate mode GUI framework |
| [nlohmann-json](https://github.com/nlohmann/json) | JSON parsing library |
| [nbnet](https://github.com/nathhB/nbnet) | Networking library |
| [Ultralight](https://ultralig.ht/) | Lightweight WebView (Desktop) |

## Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| Complex C++ memory management | Use CMake for modular management |
| WebRTC integration difficulty | Prioritize MVP (Echo Server) development |
| Difficult WASM debugging | Adopt "Desktop-first, Web-deploy" strategy |

## Documentation

- [Software Requirements Document](软件需求构思文档.md) - Complete requirements specification

## Contributing

Issues and Pull Requests are welcome!

## License

MIT License

---

*This project is for educational and learning purposes only.*
