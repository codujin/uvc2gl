#ifndef UVC2GL_RENDERER_H
#define UVC2GL_RENDERER_H

#include "Shader.h"
#include "Quad.h"
#include <memory>
namespace UVC2GL {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Disable copy
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void PreDraw(int width, int height);
    void Draw();
    void PrintOpenGLVersion();

private:
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Quad> m_quad;
};

} // namespace UVC2GL

#endif // UVC2GL_RENDERER_H
