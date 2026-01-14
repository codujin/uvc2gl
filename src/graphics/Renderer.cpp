#include "Renderer.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>

namespace UVC2GL {

Renderer::Renderer() {
    // Initialize OpenGL state if needed
}

Renderer::~Renderer() {
    // Cleanup OpenGL resources
}

void Renderer::PrintOpenGLVersion() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void Renderer::PreDraw() {
    // Currently commented out as in original code
    // glViewport(0, 0, width, height);
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw() {
    // Placeholder for actual drawing code
    // Phase 1 will add fullscreen quad rendering here
}

} // namespace UVC2GL
