#include "Window.h"
#include <stdexcept>

namespace UVC2GL {

Window::Window(const char* title, int width, int height)
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_width(width)
    , m_height(height)
    , m_shouldClose(false)
{
    InitSDL();
    CreateWindow(title);
    CreateGLContext();
    InitGLEW();
}

Window::~Window() {
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

void Window::InitSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
}

void Window::CreateWindow(const char* title) {
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_width,
        m_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
    }
    
    // Set minimum size and maintain 16:9 aspect ratio
    SDL_SetWindowMinimumSize(m_window, 640, 360);
}

void Window::CreateGLContext() {
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        throw std::runtime_error(std::string("OpenGL context creation failed: ") + SDL_GetError());
    }
}

void Window::InitGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw std::runtime_error(std::string("GLEW initialization failed: ") + 
                                 reinterpret_cast<const char*>(glewGetErrorString(err)));
    }
}

void Window::SwapBuffers() {
    SDL_GL_SwapWindow(m_window);
}

void Window::UpdateSize(int width, int height) {
    m_width = width;
    m_height = height;
}

} // namespace UVC2GL
