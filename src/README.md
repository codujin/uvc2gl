# Source Code Structure

This directory contains the modular implementation of the UVC2GL capture engine.

## Directory Structure

```
src/
├── core/           # Application lifecycle and coordination
│   ├── Application.h
│   └── Application.cpp
├── graphics/       # Rendering and window management
│   ├── Window.h
│   ├── Window.cpp
│   ├── Renderer.h
│   ├── Renderer.cpp
│   ├── Shader.h
│   ├── Shader.cpp
│   ├── Quad.h
│   └── Quad.cpp
├── assets/         # Shader files and resources
│   └── shaders/
│       ├── Quad.vert
│       └── Quad.frag
└── README.md
```

Future phases will add:
- `video/` - Video capture and decoding (Phase 2)
- `audio/` - Audio capture and playback (Phase 3)

## Current Modules (Phase 0 + Phase 1)

### Core Module (`core/`)

#### Application (`Application.h/cpp`)
- **Purpose**: Main application lifecycle and coordination
- **Responsibilities**:
  - Creates and manages Window and Renderer instances
  - Implements main game loop (input → update → render)
  - Handles application-level state and timing

### Graphics Module (`graphics/`)

#### Window (`Window.h/cpp`)
- **Purpose**: SDL2 window and OpenGL context management
- **Responsibilities**:
  - Initializes SDL2 subsystems
  - Creates window with OpenGL support
  - Manages OpenGL context creation
  - Initializes GLEW for modern OpenGL extensions
  - Handles buffer swapping
  - Provides window properties (width, height, etc.)

### Renderer (`Renderer.h/cpp`)
- **Purpose**: OpenGL rendering operations
- **Responsibilities**:
  - Pre-draw setup (viewport, clear, etc.)
  - Manages shader and quad instances
  - Coordinates rendering of fullscreen quad
  - OpenGL state management

#### Shader (`Shader.h/cpp`)
- **Purpose**: GLSL shader program management
- **Responsibilities**:
  - Loads vertex and fragment shaders from files
  - Compiles and links shader programs
  - Provides shader activation interface
  - Error handling and reporting

#### Quad (`Quad.h/cpp`)
- **Purpose**: Fullscreen quad geometry
- **Responsibilities**:
  - Creates and manages VAO/VBO for two triangles
  - Provides draw call interface
  - Handles OpenGL geometry resources

## Design Principles

- **Single Responsibility**: Each class has one clear purpose
- **Resource Management**: RAII with smart pointers for safe cleanup
- **Namespace**: All code in `UVC2GL` namespace to avoid conflicts
- **No Copy**: Classes use deleted copy constructors/operators
- **Exception Safety**: Constructors throw on failure instead of silent errors

## Future Expansion (Upcoming Phases)

### Phase 2 - Video Capture
- New `VideoCapture` class for V4L2 integration
- New `FrameDecoder` class for MJPG decoding
- Ring buffer implementation for frame storage
- Threading integration

### Phase 3 - Audio
- New `AudioCapture` class
- Audio synchronization with video

### Phase 4 - Pipeline
- Multi-threaded architecture
- Separate threads for capture, decode, render
- Advanced synchronization and buffering

## Building

The CMakeLists.txt includes all source files. Using Ninja build system:
```bash
cd build
cmake -G Ninja ..
ninja
```

Or use the VS Code build tasks (Ctrl+Shift+B).
