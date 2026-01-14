#ifndef UVC2GL_SHADER_H
#define UVC2GL_SHADER_H

#include <GL/glew.h>
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
        private:
            static std::string LoadShaderSource(const std::string& filepath);
            static GLuint CompileShader(GLenum type, const std::string& source);
            static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
            GLuint m_programID;
    };

}

#endif // UVC2GL_SHADER_H