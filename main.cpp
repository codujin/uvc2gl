#include "src/core/Application.h"
#include <iostream>
#include <exception>

int main() {
    try {
        UVC2GL::Application app("UVC2GL", 800, 600);
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}