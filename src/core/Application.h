#ifndef UVC2GL_APPLICATION_H
#define UVC2GL_APPLICATION_H

#include "../graphics/Window.h"
#include "../graphics/Renderer.h"
#include <memory>

namespace UVC2GL {

class Application {
public:
    Application(const char* title, int width, int height);
    ~Application();

    // Disable copy
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Run();

private:
    void ProcessInput();
    void Update();
    void Render();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
};

} // namespace UVC2GL

#endif // UVC2GL_APPLICATION_H
