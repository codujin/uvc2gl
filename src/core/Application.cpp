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
    
    // Query available formats
    m_availableFormats = V4L2Capabilities::QueryFormats("/dev/video1");
    
    // Try to initialize video capture (may fail if device not available)
    try {
        m_video = std::make_unique<VideoCapture>("/dev/video1", m_currentWidth, m_currentHeight, m_currentFps, 10);
        m_decoder = std::make_unique<MjpgDecoder>();
        m_video->Start();
        std::cout << "Video capture started successfully" << std::endl;
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
        m_video = std::make_unique<VideoCapture>("/dev/video1", width, height, fps, 10);
        m_video->Start();
        std::cout << "Video capture restarted successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to restart video capture: " << e.what() << std::endl;
    }
}

void Application::Render() {
    m_renderer->PreDraw( m_window->GetWidth(), m_window->GetHeight());
    m_renderer->Draw();
    RenderUI();
    m_window->SwapBuffers();
}

}// namespace UVC2GL
