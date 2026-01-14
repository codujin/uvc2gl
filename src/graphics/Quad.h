#ifndef UVC2GL_QUAD_H
#define UVC2GL_QUAD_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

namespace UVC2GL {

    class Quad{
        public:
            Quad();
            ~Quad();

            Quad(const Quad&) = delete;
            Quad& operator=(const Quad&) = delete;

            void Draw() const;

        private:
            GLuint m_VAO;
            GLuint m_VBO;
    };
}

#endif // UVC2GL_QUAD_H