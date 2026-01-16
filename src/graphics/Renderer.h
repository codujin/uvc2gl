#ifndef UVC2GL_RENDERER_H
#define UVC2GL_RENDERER_H

#include "Shader.h"
#include "Quad.h"
#include <memory>
#include <vector>
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
    void UploadVideoFrame(int width, int height, const std::vector<uint8_t>& rgb);
    float GetVideoAspectRatio() const;

private:
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Quad> m_quad;
    GLuint m_videoTexture = 0;
    int m_videoWidth = 0;
    int m_videoHeight = 0;

};

} // namespace UVC2GL

#endif // UVC2GL_RENDERER_H
