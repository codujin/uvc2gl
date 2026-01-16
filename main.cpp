#include "src/core/Application.h"
#include <iostream>
#include <exception>

int main() {
    // Force SDL to use PulseAudio before SDL initializes
    setenv("SDL_AUDIODRIVER", "pulseaudio", 1);
    
    try {
        uvc2gl::Application app("uvc2gl - Game Capture", 1920, 1080);
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}