#ifndef uvc2gl_APPLICATION_H
#define uvc2gl_APPLICATION_H

#include "../graphics/Renderer.h"
#include "../graphics/Window.h"
#include "../video/VideoCapture.h"
#include "../video/MjpgDecoder.h"
#include "../video/V4L2Capabilities.h"
#include "../audio/AudioCapture.h"
#include "../audio/AudioPlayback.h"
#include "../audio/ALSACapabilities.h"
#include "Config.h"
#include <memory>
#include <vector>

namespace uvc2gl {

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
    void SwitchAudioDevice(const std::string& deviceName);
    void ToggleFullscreen();
    void SaveConfig();

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<VideoCapture> m_video;
    std::unique_ptr<MjpgDecoder>  m_decoder;
    std::unique_ptr<AudioCapture> m_audio;
    std::unique_ptr<AudioPlayback> m_audioPlayback;
    
    std::vector<VideoDevice> m_availableDevices;
    std::vector<VideoFormat> m_availableFormats;
    std::vector<AudioDevice> m_availableAudioDevices;
    std::string m_currentDevice = "/dev/video1";
    std::string m_currentAudioDevice = "pulse";
    bool m_showContextMenu = false;
    int m_contextMenuX = 0;
    int m_contextMenuY = 0;
    int m_currentWidth = 1920;
    int m_currentHeight = 1080;
    int m_currentFps = 30;
    bool m_isFullscreen = false;
    
    AppConfig m_config;
    std::string m_configPath = "uvc2gl.conf";

};

} // namespace uvc2gl

#endif // uvc2gl_APPLICATION_H
