# UVC2GL

A Linux application that captures video from USB capture cards (UVC) and renders to OpenGL, detectable by Discord as a "game" for screen sharing.

## Current Status: Phase 0 Complete ✅

- SDL2 window with OpenGL 4.5 context
- Modular architecture ready for video capture integration
- Ninja build system

## Building

### Prerequisites
```bash
# Fedora/RHEL
sudo dnf install SDL2-devel cmake ninja-build clang

# Ubuntu/Debian
sudo apt install libsdl2-dev cmake ninja-build clang
```

### Compile
```bash
# First time setup
mkdir -p build && cd build
cmake -G Ninja ..
ninja

# Incremental builds
cd build && ninja

# Clean rebuild
rm -rf build && mkdir build && cd build
cmake -G Ninja .. && ninja
```

### Run
```bash
cd build
./UVC2GL
```

## Project Structure

```
GameCapture/
├── main.cpp            # Entry point
├── CMakeLists.txt      # Build configuration
├── plan.md             # Implementation roadmap
├── build/              # Build artifacts (Ninja)
└── src/
    ├── core/           # Application lifecycle
    │   ├── Application.h/cpp
    ├── graphics/       # Window & rendering
    │   ├── Window.h/cpp
    │   └── Renderer.h/cpp
    └── README.md       # Detailed module documentation
```

## Roadmap

See [plan.md](plan.md) for detailed implementation phases:
- **Phase 0** (✅): SDL2 window + OpenGL context
- **Phase 1**: Fullscreen quad rendering with shaders
- **Phase 2**: V4L2 video capture + MJPG decoding
- **Phase 3**: Audio capture and playback
- **Phase 4**: Multi-threaded pipeline with ring buffers