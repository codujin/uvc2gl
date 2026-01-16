#ifndef uvc2gl_WINDOW_H
#define uvc2gl_WINDOW_H
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengl.h>

namespace uvc2gl {

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    // Disable copy
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void SwapBuffers();
    void UpdateSize(int width, int height);
    bool ShouldClose() const { return m_shouldClose; }
    void SetShouldClose(bool value) { m_shouldClose = value; }
    
    SDL_Window* GetSDLWindow() const { return m_window; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    void InitSDL();
    void CreateWindow(const char* title);
    void CreateGLContext();
    void InitGLEW();
    
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    int m_width;
    int m_height;
    bool m_shouldClose;
};

} // namespace uvc2gl

#endif // uvc2gl_WINDOW_H
