#include "Shader.h"
#include <SDL_opengl.h>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace UVC2GL {
    static std::string LogInfo(GLuint obj, bool isProgram){
        GLint logLength = 0;
        if (isProgram)
            __glewGetProgramiv(obj, GL_INFO_LOG_LENGTH, &logLength);
        else
            __glewGetShaderiv(obj, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength <=1)
            return "";
        std::vector<GLchar> logBuffer(static_cast<size_t>(logLength));
        if (isProgram)
            __glewGetProgramInfoLog(obj, logLength, nullptr, logBuffer.data());
        else
            __glewGetShaderInfoLog(obj, logLength, nullptr, logBuffer.data());
        return std::string(logBuffer.data());
    }

    std::string Shader::LoadShaderSource(const std::string& filepath){
        std::ifstream fileStream(filepath);
        if (!fileStream.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filepath);
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

    GLuint Shader::CompileShader(GLenum type, const std::string& source){
        GLuint shader = __glewCreateShader(type);
        const char* src = source.c_str();
        __glewShaderSource(shader, 1, &src, nullptr);
        __glewCompileShader(shader);

        GLint success;
        __glewGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::string log = LogInfo(shader, false);
            __glewDeleteShader(shader);
            throw std::runtime_error("Shader compilation failed: " + log);
        }
        return shader;
    }

    GLuint Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader){
        GLuint program = __glewCreateProgram();
        __glewAttachShader(program, vertexShader);
        __glewAttachShader(program, fragmentShader);
        __glewLinkProgram(program);

        GLint success;
        __glewGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            std::string log = LogInfo(program, true);
            __glewDeleteProgram(program);
            throw std::runtime_error("Program linking failed: " + log);
        }
        return program;
    }

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource = LoadShaderSource(vertexPath);
        std::string fragmentSource = LoadShaderSource(fragmentPath);

        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        m_programID = LinkProgram(vertexShader, fragmentShader);
        __glewDetachShader(m_programID, vertexShader);
        __glewDetachShader(m_programID, fragmentShader);

        __glewDeleteShader(vertexShader);
        __glewDeleteShader(fragmentShader);
    }

    Shader::~Shader() {
        if(m_programID) 
          __glewDeleteProgram(m_programID);
    }

    void Shader::Use() const {
        __glewUseProgram(m_programID);
    }

    void UVC2GL::Shader::SetInt(const std::string& name, int v) const {
    GLint loc = glGetUniformLocation(m_programID, name.c_str());
    if (loc >= 0) {
        glUniform1i(loc, v);
    }
}

}