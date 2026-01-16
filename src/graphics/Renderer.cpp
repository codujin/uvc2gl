#include "Renderer.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>

static void InitTexture(GLuint& tex) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


namespace uvc2gl {    

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
    // Calculate viewport to maintain video aspect ratio
    float targetAspect = GetVideoAspectRatio();
    if (targetAspect <= 0.0f) {
        // No video loaded yet, default to 16:9
        targetAspect = 16.0f / 9.0f;
    }
    
    float windowAspect = static_cast<float>(width) / static_cast<float>(height);
    
    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = width;
    int viewportHeight = height;
    
    if (windowAspect > targetAspect) {
        // Window is too wide, add pillarbox (black bars on sides)
        viewportWidth = static_cast<int>(height * targetAspect);
        viewportX = (width - viewportWidth) / 2;
    } else if (windowAspect < targetAspect) {
        // Window is too tall, add letterbox (black bars on top/bottom)
        viewportHeight = static_cast<int>(width / targetAspect);
        viewportY = (height - viewportHeight) / 2;
    }
    
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::UploadVideoFrame(int width, int height, const std::vector<uint8_t>& rgb) {
    if (width <= 0 || height <= 0 || rgb.empty()) {
        return;
    }
    
    size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
    if (rgb.size() != expected_size) {
        std::cerr << "Warning: RGB data size mismatch. Expected " << expected_size 
                  << " but got " << rgb.size() << std::endl;
        return;
    }
    
    if (m_videoTexture == 0) {
        InitTexture(m_videoTexture);
    }

    glBindTexture(GL_TEXTURE_2D, m_videoTexture);

    if (width != m_videoWidth || height != m_videoHeight) {
        m_videoWidth = width;
        m_videoHeight = height;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            rgb.data()
        );
    } else {
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            width,
            height,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            rgb.data()
        );
    }
}


void Renderer::Draw() {
    m_shader->Use();
    if (m_videoTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_videoTexture);
        m_shader->SetInt("uTex", 0);
        m_shader->SetInt("uFlipY", 1); // Flip Y for video textures
    }
    m_quad->Draw();
}

float Renderer::GetVideoAspectRatio() const {
    if (m_videoWidth > 0 && m_videoHeight > 0) {
        return static_cast<float>(m_videoWidth) / static_cast<float>(m_videoHeight);
    }
    return 0.0f;
}

} // namespace uvc2gl
