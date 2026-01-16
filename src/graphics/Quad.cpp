#include "Quad.h"
#include <stdexcept>


namespace uvc2gl {

    Quad::Quad(){
        // Render two Tris to make a Quad
        const float verts[]= {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            //
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };

        __glewGenVertexArrays(1, &m_VAO);
        __glewGenBuffers(1, &m_VBO);
        __glewBindVertexArray(m_VAO);
        __glewBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        __glewBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        __glewBindBuffer(GL_ARRAY_BUFFER, 0);
        __glewBindVertexArray(0);

        if (glGetError() != GLEW_OK) {
            throw std::runtime_error("Failed to create Quad geometry");
        }
    }

    Quad::~Quad(){
        __glewDeleteVertexArrays(1, &m_VAO);
        __glewDeleteBuffers(1, &m_VBO);
    }

    void Quad::Draw() const{
        __glewBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        __glewBindVertexArray(0);
    }

} // namespace uvc2gl