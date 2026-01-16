#ifndef UVC2GL_SHADER_H
#define UVC2GL_SHADER_H

#include <GL/glew.h>
#include <string>
#include <SDL2/SDL_opengl.h>
#include <string>

namespace UVC2GL {
    class Shader {
        public:
            Shader(const std::string& vertexPath, const std::string& fragmentPath);
            ~Shader();

            Shader(const Shader&) = delete;
            Shader& operator=(const Shader&) = delete;

            void Use() const;
            unsigned int GetID() const;
            void SetInt(const std::string& name, int v) const;

        private:
            static std::string LoadShaderSource(const std::string& filepath);
            static GLuint CompileShader(GLenum type, const std::string& source);
            static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
            GLuint m_programID;
    };

}

#endif // UVC2GL_SHADER_H