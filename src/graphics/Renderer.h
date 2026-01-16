#ifndef uvc2gl_RENDERER_H
#define uvc2gl_RENDERER_H

#include "Shader.h"
#include "Quad.h"
#include <memory>
#include <vector>
namespace uvc2gl {

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

} // namespace uvc2gl

#endif // uvc2gl_RENDERER_H
