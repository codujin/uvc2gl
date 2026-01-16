#include "src/core/Application.h"
#include "src/core/Version.h"
#include <iostream>
#include <exception>
#include <cstring>

int main(int argc, char* argv[]) {
    // Handle version flag
    if (argc > 1 && (std::strcmp(argv[1], "--version") == 0 || std::strcmp(argv[1], "-v") == 0)) {
        std::cout << "uvc2gl version " << uvc2gl::VERSION << std::endl;
        return 0;
    }
    
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