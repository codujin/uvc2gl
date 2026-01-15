#include "Application.h"
#include <iostream>

namespace UVC2GL {

Application::Application(const char* title, int width, int height) {
    m_window = std::make_unique<Window>(title, width, height);
    m_renderer = std::make_unique<Renderer>();
    
    // Print OpenGL version info after context creation
    m_renderer->PrintOpenGLVersion();
}

Application::~Application() {
    // Unique pointers will automatically clean up
}

void Application::Run() {
    while (!m_window->ShouldClose()) {
        ProcessInput();
        Update();
        Render();
    }
}

void Application::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            std::cout << "Quitting application." << std::endl;
            m_window->SetShouldClose(true);
        }
    }
}

void Application::Update() {
    // Future: Update game logic, handle timing, etc.
}

void Application::Render() {
    m_renderer->PreDraw( m_window->GetWidth(), m_window->GetHeight());
    m_renderer->Draw();
    m_window->SwapBuffers();
}

} // namespace UVC2GL
