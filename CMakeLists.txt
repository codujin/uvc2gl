cmake_minimum_required(VERSION 3.12)
project(UVC2GL)

# compiler
set(CMAKE_C_COMPILER clang-21)
set(CMAKE_CXX_COMPILER clang++-21)
set(CMAKE_CXX_STANDARD 23)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Source files
set(SOURCES
    main.cpp
    src/core/Application.cpp
    src/graphics/Window.cpp
    src/graphics/Renderer.cpp
    src/graphics/Quad.cpp
    src/graphics/Shader.cpp
)

# executable
add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(Probe src/video/v4l2Probe.cpp)
add_executable(StreamMjpg src/video/v4l2StreamMjpg.cpp)

# Copy shader files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/assets/shaders
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying shaders to build directory"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} OpenGL::GL GLEW::GLEW)
