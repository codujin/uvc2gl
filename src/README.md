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
├── video/          # Video capture and decoding
│   ├── VideoCapture.h
│   ├── VideoCapture.cpp
│   ├── MjpgDecoder.h
│   ├── MjpgDecoder.cpp
│   ├── V4L2Capabilities.h
│   ├── V4L2Capabilities.cpp
│   ├── Frame.h
│   ├── RingBuffer.h
│   ├── v4l2Probe.cpp
│   └── v4l2StreamMjpg.cpp
├── assets/         # Shader files and resources
│   └── shaders/
│       ├── Quad.vert
│       └── Quad.frag
└── README.md
```

## Current Modules

### Core Module (`core/`)

#### Application (`Application.h/cpp`)
- **Purpose**: Main application lifecycle and coordination
- **Responsibilities**:
  - Creates and manages Window, Renderer, and VideoCapture instances
  - Implements main game loop (input → update → render)
  - Handles Dear ImGui integration for UI
  - Enumerates and manages multiple video devices
  - Supports runtime device switching
  - Manages video format switching
  - Coordinates frame retrieval and upload to GPU

### Graphics Module (`graphics/`)

#### Window (`Window.h/cpp`)
- **Purpose**: SDL2 window and OpenGL context management
- **Responsibilities**:
  - Initializes SDL2 subsystems
  - Creates resizable window with OpenGL support
  - Manages OpenGL context creation
  - Initializes GLEW for modern OpenGL extensions
  - Handles buffer swapping
  - Provides window properties (width, height, etc.)

#### Renderer (`Renderer.h/cpp`)
- **Purpose**: OpenGL rendering operations
- **Responsibilities**:
  - Pre-draw setup with dynamic aspect ratio calculation
  - Manages shader and quad instances
  - Uploads video frames as OpenGL textures
  - Renders fullscreen quad with video texture
  - OpenGL state management
  - Letterbox/pillarbox handling for aspect ratio

#### Shader (`Shader.h/cpp`)
- **Purpose**: GLSL shader program management
- **Responsibilities**:
  - Loads vertex and fragment shaders from files
  - Compiles and links shader programs
  - Provides uniform variable setting
  - Error handling and reporting

#### Quad (`Quad.h/cpp`)
- **Purpose**: Fullscreen quad geometry
- **Responsibilities**:
  - Creates and manages VAO/VBO for two triangles
  - Provides draw call interface
  - Handles OpenGL geometry resources

### Video Module (`video/`)

#### VideoCapture (`VideoCapture.h/cpp`)
- **Purpose**: V4L2 video capture with background thread
- **Responsibilities**:
  - Opens and configures V4L2 device
  - Manages memory-mapped buffers
  - Runs capture loop in separate thread
  - Decodes MJPEG frames to RGB
  - Pushes decoded frames to ring buffer
  - Handles device errors and cleanup

#### MjpgDecoder (`MjpgDecoder.h/cpp`)
- **Purpose**: FFmpeg-based MJPEG decoding
- **Responsibilities**:
  - Initializes FFmpeg MJPEG codec
  - Decodes MJPEG data to raw video frames
  - Converts YUV to RGB using swscale
  - Manages codec context and frame buffers
  - Validates MJPEG data integrity

#### V4L2Capabilities (`V4L2Capabilities.h/cpp`)
- **Purpose**: Query devices and available video formats
- **Responsibilities**:
  - Enumerates all V4L2 video capture devices in /dev
  - Queries device capabilities (name, driver)
  - Enumerates supported resolutions for each device
  - Queries available framerates for each resolution
  - Returns list of VideoDevice and VideoFormat structs
  - Used for populating UI device and format menus

#### Frame (`Frame.h`)
- **Purpose**: Frame data structure
- **Contains**: Width, height, RGB data vector, timestamp

#### RingBuffer (`RingBuffer.h`)
- **Purpose**: Thread-safe circular buffer for frames
- **Responsibilities**:
  - Lock-free push/pop operations with mutex
  - Always returns latest frame (no queue buildup)
  - Handles frame overflow gracefully

#### Utilities
- **v4l2Probe.cpp**: Standalone tool to query V4L2 device info
- **v4l2StreamMjpg.cpp**: Test utility to capture MJPEG frames to disk

## Design Principles

- **Single Responsibility**: Each class has one clear purpose
- **Resource Management**: RAII with smart pointers for safe cleanup
- **Namespace**: All code in `UVC2GL` namespace to avoid conflicts
- **No Copy**: Classes use deleted copy constructors/operators
- **Exception Safety**: Constructors throw on failure instead of silent errors
- **Thread Safety**: Mutex-protected ring buffer for cross-thread communication
- **Lock-free Latest Frame**: Ring buffer always returns most recent frame

## Architecture Overview

### Threading Model
- **Main Thread**: SDL event loop, ImGui rendering, OpenGL texture upload
- **Capture Thread**: V4L2 capture, MJPEG decoding, ring buffer push

### Data Flow
```
V4L2 Device → MJPEG Buffers → FFmpeg Decoder → RGB Frame → Ring Buffer → GPU Texture → OpenGL Quad
    (capture thread)                                            (main thread)
```

### Synchronization
- Ring buffer uses mutex for thread-safe access
- Main thread polls for latest frame each render loop
- No blocking - if no new frame, renders previous frame

## Future Expansion

### Phase 3 - Audio
- Audio capture for ALSA/PulseAudio
- Audio synchronization with video timestamps
- Audio output through SDL2
- Advanced synchronization and buffering

## Building

The CMakeLists.txt includes all source files. Using Ninja build system:
```bash
cd build
cmake -G Ninja ..
ninja
```

Or use the VS Code build tasks (Ctrl+Shift+B).
