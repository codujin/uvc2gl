#include "Renderer.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>

namespace UVC2GL {

Renderer::Renderer() {
    m_shader = std::make_unique<Shader>("shaders/Quad.vert", "shaders/Quad.frag");
    m_quad = std::make_unique<Quad>();
    glDisable(GL_DEPTH_TEST);
}

Renderer::~Renderer() = default;

void Renderer::PrintOpenGLVersion() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void Renderer::PreDraw(int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw() {
    m_shader->Use();
    m_quad->Draw();
}

} // namespace UVC2GL
