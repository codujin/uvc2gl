#ifndef UVC2GL_RENDERER_H
#define UVC2GL_RENDERER_H

namespace UVC2GL {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Disable copy
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void PreDraw();
    void Draw();
    void PrintOpenGLVersion();

private:
    // Future: Add shader programs, VAOs, textures, etc.
};

} // namespace UVC2GL

#endif // UVC2GL_RENDERER_H
