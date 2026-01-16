# Source Code Structure

This directory contains the modular implementation of the uvc2gl capture engine.

## Directory Structure

```
src/
├── core/           # Application lifecycle and configuration
│   ├── Application.h
│   ├── Application.cpp
│   └── Config.h
├── graphics/       # Rendering and window management
│   ├── Window.h
│   ├── Window.cpp
│   ├── Renderer.h
│   ├── Renderer.cpp
│   ├── Shader.h
│   ├── Shader.cpp
│   ├── Quad.h
│   └── Quad.cpp
├── audio/          # Audio capture and playback
│   ├── AudioCapture.h
│   ├── AudioCapture.cpp
│   ├── AudioPlayback.h
│   ├── AudioPlayback.cpp
│   ├── ALSACapabilities.h
│   └── ALSACapabilities.cpp
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
  - Creates and manages Window, Renderer, VideoCapture, and Audio instances
  - Implements main game loop (input → update → render)
  - Handles Dear ImGui integration for collapsible UI menus
  - Enumerates and manages multiple video and audio devices
  - Supports runtime device switching with validation
  - Manages video format switching
  - Coordinates frame retrieval and upload to GPU
  - Audio volume control via ImGui slider
  - Fullscreen toggle (F11/F/ESC)
  - Configuration persistence

#### Config (`Config.h`)
- **Purpose**: Configuration file management
- **Responsibilities**:
  - Saves/loads device preferences (video/audio)
  - Persists resolution, framerate, and volume settings
  - Simple key=value format (uvc2gl.conf)
  - Validates settings on load and falls back to defaults

### Graphics Module (`graphics/`)

#### Window (`Window.h/cpp`)
- **Purpose**: SDL2 window and OpenGL context management
- **Responsibilities**:
  - Initializes SDL2 subsystems (video and audio)
  - Creates resizable window with OpenGL support
  - Manages OpenGL context creation
  - Initializes GLEW for modern OpenGL extensions
  - Handles buffer swapping and fullscreen mode
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

### Audio Module (`audio/`)

#### AudioCapture (`AudioCapture.h/cpp`)
- **Purpose**: ALSA audio capture with background thread
- **Responsibilities**:
  - Opens and configures ALSA PCM device
  - Manages double-buffered audio frames
  - Runs capture loop in separate thread
  - Handles sample rate and period size adjustments
  - Thread-safe buffer swapping
  - Device error recovery

#### AudioPlayback (`AudioPlayback.h/cpp`)
- **Purpose**: SDL2 audio playback with ring buffer
- **Responsibilities**:
  - Opens SDL2 audio device for playback
  - Manages ring buffer for audio samples
  - SDL audio callback for filling audio stream
  - Real-time volume control (0.0-1.0 scale)
  - Thread-safe sample queuing

#### ALSACapabilities (`ALSACapabilities.h/cpp`)
- **Purpose**: Query ALSA devices
- **Responsibilities**:
  - Enumerates all ALSA PCM capture devices
  - Returns device name and description
  - Used for populating audio device menu

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
  - Exception-safe destruction and stopping

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
- **Namespace**: All code in `uvc2gl` namespace to avoid conflicts
- **No Copy**: Classes use deleted copy constructors/operators
- **Exception Safety**: Constructors throw on failure, destructors catch all exceptions
- **Thread Safety**: Mutex-protected buffers for cross-thread communication
- **Lock-free Latest Frame**: Ring buffer always returns most recent frame
- **Robust Error Handling**: Comprehensive try-catch blocks, device validation

## Architecture Overview

### Threading Model
- **Main Thread**: SDL event loop, ImGui rendering, OpenGL texture upload, audio queuing
- **Video Capture Thread**: V4L2 capture, MJPEG decoding, ring buffer push
- **Audio Capture Thread**: ALSA capture, double-buffer swapping
- **SDL Audio Thread**: Audio playback callback, ring buffer consumption

### Data Flow
```
V4L2 Device → MJPEG Buffers → FFmpeg Decoder → RGB Frame → Ring Buffer → GPU Texture → OpenGL Quad
    (video capture thread)                                       (main thread)

ALSA Device → PCM Samples → Double Buffer → Main Thread → SDL Ring Buffer → Audio Playback
    (audio capture thread)                  (main thread)         (SDL audio thread)
```

### Synchronization
- Video: Ring buffer uses mutex for thread-safe access
- Audio: Double-buffered frames with mutex protection, consumed after read
- Main thread polls for latest frames each render loop
- No blocking - if no new frame, renders/plays previous data

## Recent Improvements

- **Audio Support**: Full ALSA capture with SDL2 playback
- **Volume Control**: Real-time adjustable volume with ImGui slider
- **Configuration Persistence**: Auto-saves device preferences and settings
- **Device Validation**: Checks if devices are actually working before use
- **Fullscreen Mode**: F11/F/ESC support with proper window management
- **Collapsible UI**: Organized Video/Audio sections in context menu
- **Robust Error Handling**: Exception-safe destructors and device switching
- **Audio Distortion Fix**: Proper buffer management to prevent re-reading old samples

## Building

The CMakeLists.txt includes all source files. Using Ninja build system:
```bash
cd build
cmake -G Ninja ..
ninja
```

Or use the VS Code build tasks (Ctrl+Shift+B).
