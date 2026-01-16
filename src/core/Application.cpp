#include "Application.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace uvc2gl {

Application::Application(const char* title, int width, int height) {
    m_window = std::make_unique<Window>(title, width, height);
    m_renderer = std::make_unique<Renderer>();
    
    // Print OpenGL version info after context creation
    m_renderer->PrintOpenGLVersion();
    
    // Initialize ImGui
    InitImGui();
    
    // Load config
    m_config.LoadFromFile(m_configPath);
    
    // Enumerate available devices
    m_availableDevices = V4L2Capabilities::EnumerateDevices();
    
    // Try to use saved device if available, otherwise fallback
    m_currentDevice = m_config.videoDevice;
    bool deviceFound = false;
    
    if (!m_availableDevices.empty()) {
        for (const auto& device : m_availableDevices) {
            if (device.path == m_config.videoDevice) {
                deviceFound = true;
                break;
            }
        }
        
        if (!deviceFound) {
            // Saved device not found, use first available
            m_currentDevice = m_availableDevices[0].path;
            std::cout << "Saved device not found, using " << m_currentDevice << std::endl;
        }
        std::cout << "Found " << m_availableDevices.size() << " video device(s)" << std::endl;
    }
    
    // Query available formats for current device
    m_availableFormats = V4L2Capabilities::QueryFormats(m_currentDevice);
    
    // Try to use saved resolution/fps if available
    m_currentWidth = m_config.width;
    m_currentHeight = m_config.height;
    m_currentFps = m_config.fps;
    
    bool formatFound = false;
    if (!m_availableFormats.empty()) {
        for (const auto& format : m_availableFormats) {
            if (format.width == m_config.width && 
                format.height == m_config.height && 
                format.fps == m_config.fps) {
                formatFound = true;
                break;
            }
        }
        
        if (!formatFound) {
            // Saved format not available, use first available
            m_currentWidth = m_availableFormats[0].width;
            m_currentHeight = m_availableFormats[0].height;
            m_currentFps = m_availableFormats[0].fps;
            std::cout << "Saved format not available, using " << m_currentWidth << "x" 
                      << m_currentHeight << "@" << m_currentFps << "fps" << std::endl;
        }
    }
    
    // Try to initialize video capture (may fail if device not available)
    try {
        m_video = std::make_unique<VideoCapture>(m_currentDevice, m_currentWidth, m_currentHeight, m_currentFps, 10);
        m_decoder = std::make_unique<MjpgDecoder>();
        m_video->Start();
        
        // Give it a moment to start up and validate it's actually working
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        if (!m_video->IsRunning()) {
            std::cerr << "Video capture failed to start properly, resetting" << std::endl;
            m_video.reset();
            m_decoder.reset();
        } else {
            std::cout << "Video capture started on " << m_currentDevice << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize video capture: " << e.what() << std::endl;
        std::cerr << "Running without video input." << std::endl;
        m_video.reset();
        m_decoder.reset();
    }
    
    // Initialize audio capture
    m_availableAudioDevices = ALSACapabilities::EnumerateDevices();
    
    // Try to use saved audio device if available
    m_currentAudioDevice = m_config.audioDevice;
    bool audioDeviceFound = false;
    
    for (const auto& device : m_availableAudioDevices) {
        if (device.name == m_config.audioDevice) {
            audioDeviceFound = true;
            break;
        }
    }
    
    if (!audioDeviceFound && !m_availableAudioDevices.empty()) {
        // Saved audio device not found, fallback to pulse or first device
        m_currentAudioDevice = "pulse";
        for (const auto& device : m_availableAudioDevices) {
            if (device.description.find("USB3 Video") != std::string::npos ||
                device.description.find("USB 3 Video") != std::string::npos) {
                m_currentAudioDevice = device.name;
                std::cout << "Found capture card audio: " << device.description << std::endl;
                break;
            }
        }
    }
    
    try {
        m_audio = std::make_unique<AudioCapture>(m_currentAudioDevice, 48000, 2, 1024);
        m_audio->Start();
        std::cout << "Audio capture started on " << m_currentAudioDevice << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize audio capture: " << e.what() << std::endl;
        std::cerr << "Running without audio input." << std::endl;
    }
    
    // Initialize audio playback
    try {
        m_audioPlayback = std::make_unique<AudioPlayback>(48000, 2);
        m_audioPlayback->Start();
        // Restore saved volume
        m_audioPlayback->SetVolume(m_config.volume);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize audio playback: " << e.what() << std::endl;
        std::cerr << "Running without audio output." << std::endl;
    }
}

Application::~Application() {
    std::cout << "Shutting down application..." << std::endl;
    
    // Save config before shutting down
    try {
        SaveConfig();
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
    }
    
    // Stop audio playback and capture safely
    try {
        if (m_audioPlayback) {
            m_audioPlayback->Stop();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping audio playback: " << e.what() << std::endl;
    }
    
    try {
        if (m_audio) {
            m_audio->Stop();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping audio capture: " << e.what() << std::endl;
    }
    
    try {
        if (m_video) {
            m_video->Stop();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping video capture: " << e.what() << std::endl;
    }
    
    try {
        ShutdownImGui();
    } catch (const std::exception& e) {
        std::cerr << "Error shutting down ImGui: " << e.what() << std::endl;
    }
    
    // Unique pointers will automatically clean up
}

void Application::Run() {
    std::cout << "Entering main loop..." << std::endl;
    while (!m_window->ShouldClose()) {
        ProcessInput();
        Update();
        Render();
    }
    std::cout << "Exiting main loop." << std::endl;
}

void Application::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        
        if (event.type == SDL_QUIT) {
            std::cout << "Quitting application." << std::endl;
            m_window->SetShouldClose(true);
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_RIGHT) {
                m_showContextMenu = true;
                m_contextMenuX = event.button.x;
                m_contextMenuY = event.button.y;
            } else if (event.button.button == SDL_BUTTON_LEFT) {
                // Close menu only if clicking outside of ImGui windows
                if (m_showContextMenu && !ImGui::GetIO().WantCaptureMouse) {
                    m_showContextMenu = false;
                }
            }
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (m_showContextMenu) {
                    m_showContextMenu = false;
                } else if (m_isFullscreen) {
                    ToggleFullscreen();
                }
            } else if (event.key.keysym.sym == SDLK_f || event.key.keysym.sym == SDLK_F11) {
                ToggleFullscreen();
            }
        } else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int newWidth = event.window.data1;
                int newHeight = event.window.data2;
                m_window->UpdateSize(newWidth, newHeight);
            }
        }
    }
}

void Application::Update() {
    // Get the latest frame from video capture
    if (m_video) {
        auto frameOpt = m_video->GetFrame();
        if (frameOpt.has_value()) {
            auto& frame = frameOpt.value();
            
            // Frame is already decoded to RGB in the capture thread
            // Just upload directly to GPU
            if (!frame.data.empty() && frame.width > 0 && frame.height > 0) {
                m_renderer->UploadVideoFrame(frame.width, frame.height, frame.data);
            }
        }
    }
    
    // Get and play audio
    if (m_audio && m_audioPlayback) {
        AudioFrame audioFrame;
        if (m_audio->GetAudioFrame(audioFrame)) {
            if (!audioFrame.samples.empty()) {
                m_audioPlayback->QueueAudio(audioFrame.samples.data(), audioFrame.frameCount);
            }
        }
    }
}

void Application::InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL2_InitForOpenGL(m_window->GetSDLWindow(), SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL3_Init("#version 460");
}

void Application::ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Application::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    if (m_showContextMenu) {
        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_contextMenuX), static_cast<float>(m_contextMenuY)));
        if (ImGui::Begin("##contextmenu", &m_showContextMenu, 
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            
            // Video section
            if (ImGui::CollapsingHeader("Video")) {
                if (!m_availableDevices.empty()) {
                    ImGui::Text("Device");
                    ImGui::Indent();
                    for (const auto& device : m_availableDevices) {
                        bool isSelected = (device.path == m_currentDevice);
                        if (ImGui::MenuItem(device.toString().c_str(), nullptr, isSelected)) {
                            if (!isSelected) {
                                SwitchDevice(device.path);
                            }
                        }
                    }
                    ImGui::Unindent();
                    ImGui::Spacing();
                }
                
                ImGui::Text("Resolution & Framerate");
                ImGui::Indent();
                for (const auto& format : m_availableFormats) {
                    std::string label = format.toString();
                    if (ImGui::MenuItem(label.c_str())) {
                        RestartCapture(format.width, format.height, format.fps);
                        m_showContextMenu = false;
                    }
                }
                ImGui::Unindent();
                ImGui::Spacing();
                ImGui::Text("Current: %dx%d @ %dfps", m_currentWidth, m_currentHeight, m_currentFps);
            }
            
            // Audio section
            if (ImGui::CollapsingHeader("Audio")) {
                if (!m_availableAudioDevices.empty()) {
                    ImGui::Text("Device");
                    ImGui::Indent();
                    for (const auto& device : m_availableAudioDevices) {
                        bool isSelected = (device.name == m_currentAudioDevice);
                        if (ImGui::MenuItem(device.toString().c_str(), nullptr, isSelected)) {
                            if (!isSelected) {
                                SwitchAudioDevice(device.name);
                            }
                        }
                    }
                    ImGui::Unindent();
                    ImGui::Spacing();
                }
                
                if (m_audioPlayback) {
                    ImGui::Text("Volume");
                    ImGui::Indent();
                    float volume = m_audioPlayback->GetVolume() * 100.0f;
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##volume", &volume, 0.0f, 100.0f, "%.0f%%", ImGuiSliderFlags_None)) {
                        m_audioPlayback->SetVolume(volume / 100.0f);
                        // Save config when user releases the slider
                        if (!ImGui::IsItemActive()) {
                            SaveConfig();
                        }
                    }
                    ImGui::Unindent();
                }
            }
            
            // Fullscreen toggle
            ImGui::Separator();
            if (ImGui::MenuItem(m_isFullscreen ? "Exit Fullscreen (F11/ESC)" : "Fullscreen (F11)")) {
                ToggleFullscreen();
                m_showContextMenu = false;
            }
            
            ImGui::End();
        }
    }
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::RestartCapture(int width, int height, int fps) {
    std::cout << "Restarting capture with " << width << "x" << height << " @ " << fps << "fps" << std::endl;
    
    // Stop current capture safely
    if (m_video) {
        try {
            m_video->Stop();
        } catch (const std::exception& e) {
            std::cerr << "Error stopping video: " << e.what() << std::endl;
        }
        m_video.reset();
    }
    
    // Update current settings
    m_currentWidth = width;
    m_currentHeight = height;
    m_currentFps = fps;
    
    // Start new capture
    try {
        m_video = std::make_unique<VideoCapture>(m_currentDevice, width, height, fps, 10);
        m_video->Start();
        
        // Give it a moment to validate it's working
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        if (!m_video->IsRunning()) {
            std::cerr << "Video capture failed to restart" << std::endl;
            m_video.reset();
        } else {
            std::cout << "Video capture restarted successfully" << std::endl;
            SaveConfig();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to restart video capture: " << e.what() << std::endl;
        m_video.reset();
    }
}

void Application::SwitchDevice(const std::string& devicePath) {
    if (devicePath == m_currentDevice) {
        return;
    }
    
    std::cout << "Switching to device: " << devicePath << std::endl;
    
    // Query formats for new device first, before stopping anything
    std::vector<VideoFormat> newFormats = V4L2Capabilities::QueryFormats(devicePath);
    
    if (newFormats.empty()) {
        std::cerr << "No MJPEG formats available for device " << devicePath << std::endl;
        return;
    }
    
    // Stop current capture safely
    if (m_video) {
        try {
            m_video->Stop();
        } catch (const std::exception& e) {
            std::cerr << "Error stopping video: " << e.what() << std::endl;
        }
        m_video.reset();
    }
    
    // Reset decoder to clear any cached state
    m_decoder.reset();
    
    // Give the system a moment to fully release the device
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Update device and formats
    m_currentDevice = devicePath;
    m_availableFormats = std::move(newFormats);
    
    // Use first available format from the new device
    m_currentWidth = m_availableFormats[0].width;
    m_currentHeight = m_availableFormats[0].height;
    m_currentFps = m_availableFormats[0].fps;
    
    // Recreate decoder for new device
    m_decoder = std::make_unique<MjpgDecoder>();
    
    // Start capture with new device
    try {
        m_video = std::make_unique<VideoCapture>(m_currentDevice, m_currentWidth, m_currentHeight, m_currentFps, 10);
        m_video->Start();
        
        // Give it a moment to validate it's working
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        if (!m_video->IsRunning()) {
            std::cerr << "Video capture failed to start on " << m_currentDevice << std::endl;
            m_video.reset();
        } else {
            std::cout << "Successfully switched to " << m_currentDevice << " at " 
                      << m_currentWidth << "x" << m_currentHeight << "@" << m_currentFps << std::endl;
            SaveConfig();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to switch device: " << e.what() << std::endl;
        m_video.reset();
    }
}

void Application::SwitchAudioDevice(const std::string& deviceName) {
    if (deviceName == m_currentAudioDevice) {
        return;
    }
    
    std::cout << "Switching to audio device: " << deviceName << std::endl;
    
    // Stop current audio capture safely
    if (m_audio) {
        try {
            m_audio->Stop();
        } catch (const std::exception& e) {
            std::cerr << "Error stopping audio: " << e.what() << std::endl;
        }
        m_audio.reset();
    }
    
    // Update device
    m_currentAudioDevice = deviceName;
    
    // Start capture with new audio device
    try {
        m_audio = std::make_unique<AudioCapture>(m_currentAudioDevice, 48000, 2, 1024);
        m_audio->Start();
        std::cout << "Successfully switched to audio device: " << m_currentAudioDevice << std::endl;
        SaveConfig();
    } catch (const std::exception& e) {
        std::cerr << "Failed to switch audio device: " << e.what() << std::endl;
        m_audio.reset();
    }
}

void Application::ToggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    
    SDL_Window* sdlWindow = m_window->GetSDLWindow();
    if (m_isFullscreen) {
        SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        std::cout << "Entered fullscreen mode" << std::endl;
    } else {
        SDL_SetWindowFullscreen(sdlWindow, 0);
        std::cout << "Exited fullscreen mode" << std::endl;
    }
}

void Application::SaveConfig() {
    m_config.videoDevice = m_currentDevice;
    m_config.audioDevice = m_currentAudioDevice;
    m_config.width = m_currentWidth;
    m_config.height = m_currentHeight;
    m_config.fps = m_currentFps;
    if (m_audioPlayback) {
        m_config.volume = m_audioPlayback->GetVolume();
    }
    
    if (m_config.SaveToFile(m_configPath)) {
        std::cout << "Config saved" << std::endl;
    }
}

void Application::Render() {
    m_renderer->PreDraw( m_window->GetWidth(), m_window->GetHeight());
    m_renderer->Draw();
    RenderUI();
    m_window->SwapBuffers();
}

}// namespace uvc2gl
