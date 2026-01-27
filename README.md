# uvc2gl

A Linux application that captures video and audio from USB capture cards (UVC/V4L2 + ALSA) and renders to OpenGL with real-time format switching via GUI.

## Features

- **Multi-Device Support**: Switch between multiple video and audio capture devices at runtime
- **Dual Format Support**: MJPEG and YUYV video format support with runtime switching
- **V4L2 Video Capture**: Direct capture from USB capture devices with V4L2
- **ALSA Audio Capture**: Real-time audio capture with SDL2 playback
- **Optimized Decoding**: 
  - FFmpeg-based MJPEG hardware decoding
  - Custom YUYV decoder with ITU-R BT.601 color space conversion
  - Achieves 60fps at 1080p on modern CPUs with compiler auto-vectorization
- **OpenGL Rendering**: Modern OpenGL 4.6 with custom shaders
- **Live Format Switching**: Right-click context menu to change resolution/framerate/format/devices
- **Audio Volume Control**: Adjustable volume with real-time slider
- **Configuration Persistence**: Saves device preferences, resolution, framerate, format, and volume settings
- **Fullscreen Support**: Press F11 or F to toggle fullscreen mode
- **Auto-detected Formats**: Queries available formats from each capture device
- **Dynamic Aspect Ratio**: Automatically maintains correct aspect ratio for any resolution
- **Multithreaded**: Separate capture/decode threads for smooth performance
- **Dear ImGui UI**: Lightweight immediate-mode collapsible menu overlay
- **Semantic Versioning**: Auto-generated version info from VERSION file

## Building

### Prerequisites
```bash
# Fedora/RHEL
sudo dnf install SDL2-devel glew-devel cmake ninja-build clang ffmpeg-devel alsa-lib-devel libc++ libc++-devel libc++abi libc++abi-devel

# Ubuntu/Debian
sudo apt install libsdl2-dev libglew-dev cmake ninja-build clang libavcodec-dev libavformat-dev libswscale-dev libavutil-dev libasound2-dev libc++-dev libc++abi-dev
```

### Compile
```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

**Note**: Release mode is recommended for optimal performance (includes `-O3 -march=native` flags for 60fps YUYV decoding).

### Run
```bash
cd build
./uvc2gl
```

Right-click anywhere in the window to open the device/format selection menu.  
Press F11 to toggle fullscreen. Press ESC to exit fullscreen or close the menu.

## Project Structure

```
GameCapture/
├── main.cpp            # Entry point
├── CMakeLists.txt      # Build configuration
├── build/              # Build artifacts (Ninja)
│   ├── uvc2gl          # Main executable
│   ├── uvc2gl.conf     # Configuration file (auto-generated)
│   ├── Probe           # V4L2 device info utility
│   ├── StreamMjpg      # MJPEG stream capture test
│   ├── MjpgDecodeTest  # FFmpeg decoder test
│   ├── YuyvDecodeTest  # YUYV decoder test
│   └── shaders/        # Copied shader files
├── external/
│   └── imgui/          # Dear ImGui library
└── src/
    ├── core/           # Application lifecycle & config
    │   ├── Application.h/cpp
    │   └── Config.h
    ├── graphics/       # Window & rendering
    │   ├── Window.h/cpp
    │   ├── Renderer.h/cpp
    │   ├── Shader.h/cpp
    │   └── Quad.h/cpp
    ├── audio/          # Audio capture & playback
    │   ├── AudioCapture.h/cpp
    │   ├── AudioPlayback.h/cpp
    │   └── ALSACapabilities.h/cpp
    ├── video/          # Video capture & decode
    │   ├── VideoCapture.h/cpp
    │   ├── MjpgDecoder.h/cpp
    │   ├── YuyvDecoder.h/cpp
    │   ├── Frame.h
    │   ├── RingBuffer.h
    │   ├── V4L2Capabilities.h/cpp
    │   ├── v4l2Probe.cpp
    │   ├── v4l2StreamMjpg.cpp
    │   ├── MjpgDecodeTest.cpp
    │   └── YuyvDecodeTest.cpp
    └── assets/
        └── shaders/
            ├── Quad.vert
            └── Quad.frag
```

## Configuration

The application automatically saves your preferences to `uvc2gl.conf`:
- Last used video device
- Last used audio device
- Resolution and framerate
- Video format (MJPEG or YUYV)
- Audio volume level

Settings are restored on next startup. If devices are unavailable, defaults to first available device.

## Performance

- **MJPEG**: Hardware-accelerated decoding via FFmpeg (low CPU usage)
- **YUYV**: Optimized CPU-based conversion with ITU-R BT.601 color space
  - 60fps at 1920x1080 on modern CPUs with AVX2 support
  - Compiled with `-O3 -march=native` for auto-vectorization
  - Higher CPU usage than MJPEG but no hardware encoding required

## Utilities

- **Probe**: Query V4L2 device capabilities
- **StreamMjpg**: Capture raw MJPEG frames to disk
- **MjpgDecodeTest**: Test FFmpeg MJPEG decoding
- **YuyvDecodeTest**: Test YUYV decoder with known patterns

## Releases

- **Stable releases** (e.g., `v1.1.0`): Tested and ready for production use
- **Experimental releases** (e.g., `v1.2.0-yuyv.1`): Pre-release builds for testing new features
- **Dev builds**: Automatic builds from `dev` branch available as GitHub Actions artifacts

Download the latest release from the [Releases page](../../releases).