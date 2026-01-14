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

# Source files
set(SOURCES
    main.cpp
    src/core/Application.cpp
    src/graphics/Window.cpp
    src/graphics/Renderer.cpp
)

# executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} OpenGL::GL)
