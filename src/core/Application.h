#ifndef UVC2GL_APPLICATION_H
#define UVC2GL_APPLICATION_H

#include "../graphics/Renderer.h"
#include "../graphics/Window.h"
#include "../video/VideoCapture.h"
#include "../video/MjpgDecoder.h"
#include "../video/V4L2Capabilities.h"
#include <memory>
#include <vector>

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
    void RenderUI();
    void InitImGui();
    void ShutdownImGui();
    void RestartCapture(int width, int height, int fps);
    void SwitchDevice(const std::string& devicePath);

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<VideoCapture> m_video;
    std::unique_ptr<MjpgDecoder>  m_decoder;
    
    std::vector<VideoDevice> m_availableDevices;
    std::vector<VideoFormat> m_availableFormats;
    std::string m_currentDevice = "/dev/video1";
    bool m_showContextMenu = false;
    int m_contextMenuX = 0;
    int m_contextMenuY = 0;
    int m_currentWidth = 1920;
    int m_currentHeight = 1080;
    int m_currentFps = 30;

};

} // namespace UVC2GL

#endif // UVC2GL_APPLICATION_H
