#ifndef uvc2gl_QUAD_H
#define uvc2gl_QUAD_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

namespace uvc2gl {

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

#endif // uvc2gl_QUAD_H