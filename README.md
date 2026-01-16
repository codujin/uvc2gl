# UVC2GL

A Linux application that captures video from USB capture cards (UVC/V4L2) and renders to OpenGL with real-time format switching via GUI.

## Features

- **Multi-Device Support**: Switch between multiple capture cards at runtime
- **V4L2 Video Capture**: Direct MJPEG capture from USB capture devices
- **Hardware Decoding**: FFmpeg-based MJPEG to RGB decoding
- **OpenGL Rendering**: Modern OpenGL 4.6 with custom shaders
- **Live Format Switching**: Right-click context menu to change resolution/framerate
- **Auto-detected Formats**: Queries available formats from each capture device
- **Dynamic Aspect Ratio**: Automatically maintains correct aspect ratio for any resolution
- **Multithreaded**: Separate capture/decode thread for smooth performance
- **Dear ImGui UI**: Lightweight immediate-mode GUI overlay

## Building

### Prerequisites
```bash
# Fedora/RHEL
sudo dnf install SDL2-devel glew-devel cmake ninja-build clang ffmpeg-devel

# Ubuntu/Debian
sudo apt install libsdl2-dev libglew-dev cmake ninja-build clang libavcodec-dev libavformat-dev libswscale-dev libavutil-dev
```

### Compile
```bash
mkdir -p build && cd build
cmake -G Ninja ..
ninja
```

### Run
```bash
cd build
./UVC2GL
```

Right-click anywhere in the window to open the format selection menu.

## Project Structure

```
GameCapture/
├── main.cpp            # Entry point
├── CMakeLists.txt      # Build configuration
├── build/              # Build artifacts (Ninja)
│   ├── UVC2GL          # Main executable
│   ├── Probe           # V4L2 device info utility
│   ├── StreamMjpg      # MJPEG stream capture test
│   ├── MjpgDecodeTest  # FFmpeg decoder test
│   └── shaders/        # Copied shader files
├── external/
│   └── imgui/          # Dear ImGui library
└── src/
    ├── core/           # Application lifecycle
    │   └── Application.h/cpp
    ├── graphics/       # Window & rendering
    │   ├── Window.h/cpp
    │   ├── Renderer.h/cpp
    │   ├── Shader.h/cpp
    │   └── Quad.h/cpp
    ├── video/          # Video capture & decode
    │   ├── VideoCapture.h/cpp
    │   ├── MjpgDecoder.h/cpp
    │   ├── Frame.h
    │   ├── RingBuffer.h
    │   ├── V4L2Capabilities.h/cpp
    │   ├── v4l2Probe.cpp
    │   └── v4l2StreamMjpg.cpp
    └── assets/
        └── shaders/
            ├── Quad.vert
            └── Quad.frag
```

## Configuration

Default capture device: Auto-detected (prefers `/dev/video1` if available)  
Default resolution: First available format from device

The application automatically enumerates all V4L2 video capture devices on startup. Switch between devices using the right-click context menu.

## Utilities

- **Probe**: Query V4L2 device capabilities
- **StreamMjpg**: Capture raw MJPEG frames to disk
- **MjpgDecodeTest**: Test FFmpeg MJPEG decoding