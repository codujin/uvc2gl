#include "Application.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace UVC2GL {

Application::Application(const char* title, int width, int height) {
    m_window = std::make_unique<Window>(title, width, height);
    m_renderer = std::make_unique<Renderer>();
    
    // Print OpenGL version info after context creation
    m_renderer->PrintOpenGLVersion();
    
    // Initialize ImGui
    InitImGui();
    
    // Enumerate available devices
    m_availableDevices = V4L2Capabilities::EnumerateDevices();
    if (!m_availableDevices.empty()) {
        // Prefer /dev/video1 if available, otherwise use first device
        bool foundVideo1 = false;
        for (const auto& device : m_availableDevices) {
            if (device.path == "/dev/video1") {
                m_currentDevice = device.path;
                foundVideo1 = true;
                break;
            }
        }
        if (!foundVideo1) {
            m_currentDevice = m_availableDevices[0].path;
        }
        std::cout << "Found " << m_availableDevices.size() << " video device(s)" << std::endl;
    }
    
    // Query available formats for current device
    m_availableFormats = V4L2Capabilities::QueryFormats(m_currentDevice);
    
    // Use first available format if exists, otherwise keep defaults
    if (!m_availableFormats.empty()) {
        m_currentWidth = m_availableFormats[0].width;
        m_currentHeight = m_availableFormats[0].height;
        m_currentFps = m_availableFormats[0].fps;
        std::cout << "Selected default format: " << m_currentWidth << "x" << m_currentHeight 
                  << "@" << m_currentFps << "fps" << std::endl;
    }
    
    // Try to initialize video capture (may fail if device not available)
    try {
        m_video = std::make_unique<VideoCapture>(m_currentDevice, m_currentWidth, m_currentHeight, m_currentFps, 10);
        m_decoder = std::make_unique<MjpgDecoder>();
        m_video->Start();
        std::cout << "Video capture started on " << m_currentDevice << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to initialize video capture: " << e.what() << std::endl;
        std::cerr << "Running without video input." << std::endl;
    }
}

Application::~Application() {
    std::cout << "Shutting down application..." << std::endl;
    // Stop video capture first
    if (m_video) {
        m_video->Stop();
    }
    ShutdownImGui();
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
            
            // Device selection
            if (!m_availableDevices.empty()) {
                ImGui::Text("Video Device");
                ImGui::Separator();
                
                for (const auto& device : m_availableDevices) {
                    bool isSelected = (device.path == m_currentDevice);
                    if (ImGui::MenuItem(device.toString().c_str(), nullptr, isSelected)) {
                        if (!isSelected) {
                            SwitchDevice(device.path);
                        }
                    }
                }
                
                ImGui::Separator();
            }
            
            // Resolution & framerate selection
            ImGui::Text("Resolution & Framerate");
            ImGui::Separator();
            
            for (const auto& format : m_availableFormats) {
                std::string label = format.toString();
                if (ImGui::MenuItem(label.c_str())) {
                    RestartCapture(format.width, format.height, format.fps);
                    m_showContextMenu = false;
                }
            }
            
            ImGui::Separator();
            ImGui::Text("Current: %dx%d @ %dfps", m_currentWidth, m_currentHeight, m_currentFps);
            
            ImGui::End();
        }
    }
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::RestartCapture(int width, int height, int fps) {
    std::cout << "Restarting capture with " << width << "x" << height << " @ " << fps << "fps" << std::endl;
    
    // Stop current capture
    if (m_video) {
        m_video->Stop();
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
        std::cout << "Video capture restarted successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to restart video capture: " << e.what() << std::endl;
    }
}

void Application::SwitchDevice(const std::string& devicePath) {
    if (devicePath == m_currentDevice) {
        return;
    }
    
    // Stop current capture
    if (m_video) {
        m_video->Stop();
        m_video.reset();
    }
    
    // Reset decoder to clear any cached state
    if (m_decoder) {
        m_decoder.reset();
    }
    
    // Update device
    m_currentDevice = devicePath;
    
    // Query formats for new device
    m_availableFormats = V4L2Capabilities::QueryFormats(m_currentDevice);
    
    if (m_availableFormats.empty()) {
        std::cerr << "No MJPEG formats available for device " << m_currentDevice << std::endl;
        return;
    }
    
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
    } catch (const std::exception& e) {
        std::cerr << "Failed to switch device: " << e.what() << std::endl;
    }
}

void Application::Render() {
    m_renderer->PreDraw( m_window->GetWidth(), m_window->GetHeight());
    m_renderer->Draw();
    RenderUI();
    m_window->SwapBuffers();
}

}// namespace UVC2GL
