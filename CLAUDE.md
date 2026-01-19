# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PandoraMapper is an automatic mapper and database software for MUME (Multi-Users in Middle-earth). It parses MUME's XML output to determine player position and displays the map in 3D using OpenGL. The application acts as a proxy server between the game client and MUD server.

Written in mixed C/C++ with Qt framework. Original author: Azazello (2000-2009).

## Build Commands

```bash
# Generate Makefiles (run once or after .pro changes)
qmake pandora.pro

# Build release version
make release

# Build debug version
make debug

# Clean and rebuild
make release-clean && make release
```

**Output:** `release/pandoramapper`

**Dependencies:** Qt5 (xml, opengl, gui, network, core, widgets modules), OpenGL/GLU

## Running

```bash
./release/pandoramapper
# Or with custom config
./release/pandoramapper -c configs/mume.conf
# Override ports (default local: 3000, remote: 4242)
./release/pandoramapper -lp 3001 -rp 4343
```

Connect your MUD client to `localhost:3000` (mapper proxies to MUME on port 4242).

## Architecture

### Component Structure

```
src/
├── Engine/        - Core game state analyzer and mapping logic
├── Map/           - Room data models, spatial organization, persistence
├── Proxy/         - Network proxy (runs in QThread), XML parsing
├── Renderer/      - OpenGL 3D visualization (QGLWidget-based)
├── Gui/           - Qt dialogs and main window
├── GroupManager/  - Collaborative mapping over network
└── Utils/         - Configuration, XML I/O, timers
```

### Key Classes

- **CEngine** (`src/Engine/CEngine.h`): Main engine analyzing game output and managing room sync
- **Proxy** (`src/Proxy/proxy.h`): Network proxy running in separate QThread
- **CRoomManager** (`src/Map/CRoomManager.h`): Singleton holding all rooms with O(1) ID lookup
- **RendererWidget** (`src/Renderer/renderer.h`): OpenGL widget for 3D map rendering
- **GLPrimitives** (`src/Renderer/GLPrimitives.h`): Standalone OpenGL drawing functions (cone, marker)
- **CConfigurator** (`src/Utils/CConfigurator.h`): Singleton configuration manager

### Data Flow

```
MUME Server (port 4242)
    ↓
Proxy (QThread - parses XML)
    ↓
CEngine (analyzes changes)
    ↓
CRoomManager (updates rooms)
    ↓
RendererWidget (OpenGL display)
```

### Design Patterns

- **Multi-threaded Proxy**: `Proxy` extends QThread, communicates via Qt signals/slots
- **Singletons**: CConfigurator, CRoomManager for global state
- **Event-driven**: CEngine processes events from PipeManager
- **Command Queue**: Async room operations via CCommandQueue
- **Planar Organization**: Rooms organized by Z-level for rendering

## Code Conventions

- Mixed C/C++ style (legacy codebase)
- Heavy use of Qt Meta-Object System (Q_OBJECT, signals/slots)
- Manual memory management (new/delete) - be careful with ownership
- Global defines in `src/defines.h`
- UI forms in `src/Ui/*.ui` (Qt Designer files)
