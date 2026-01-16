cmake_minimum_required(VERSION 3.12)
project(uvc2gl VERSION 1.0.0 LANGUAGES C CXX)

# Version is managed in src/core/Version.h
# Update VERSION_PRERELEASE in Version.h for dev/alpha/beta builds

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libswscale libavutil)
include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIRS})

# ALSA
find_package(ALSA REQUIRED)
include_directories(${ALSA_INCLUDE_DIRS})

# ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Source files
set(SOURCES
    main.cpp
    src/core/Application.cpp
    src/graphics/Window.cpp
    src/graphics/Renderer.cpp
    src/graphics/Quad.cpp
    src/graphics/Shader.cpp
    src/video/VideoCapture.cpp
    src/video/MjpgDecoder.cpp
    src/video/V4L2Capabilities.cpp
    src/audio/AudioCapture.cpp
    src/audio/AudioPlayback.cpp
    src/audio/ALSACapabilities.cpp
    ${IMGUI_SOURCES}
)

# executable
add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(Probe src/video/v4l2Probe.cpp)
add_executable(StreamMjpg src/video/v4l2StreamMjpg.cpp)
add_executable(MjpgDecodeTest src/video/MjpgDecodeTest.cpp)
add_executable(AudioProbe src/audio/AudioProbe.cpp)

# Copy shader files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/assets/shaders
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying shaders to build directory"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} OpenGL::GL GLEW::GLEW ${FFMPEG_LINK_LIBRARIES} ${ALSA_LIBRARIES})
target_link_libraries(MjpgDecodeTest PRIVATE ${FFMPEG_LINK_LIBRARIES})
target_link_libraries(AudioProbe PRIVATE ${ALSA_LIBRARIES})
