#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets.h"
#include "non_copyable.h"

class Mesh : private NonCopyable
{
public:
    static constexpr GLuint attribute_location_position{ 0 };
    static constexpr GLuint attribute_location_normal{ 1 };
    static constexpr GLuint attribute_location_texture_coords{ 2 };

    Mesh() = delete;

    Mesh(std::vector<Vertex> const& vertices, GLenum primitive_type) : primitive_type_{ primitive_type }
    {
        glCreateVertexArrays(1, &vao_);

        glVertexArrayAttribFormat(vao_, attribute_location_position, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(vao_, attribute_location_position, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_position);

        glVertexArrayAttribFormat(vao_, attribute_location_normal, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribBinding(vao_, attribute_location_normal, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_normal);

        glVertexArrayAttribFormat(vao_, attribute_location_texture_coords, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoords));
        glVertexArrayAttribBinding(vao_, attribute_location_texture_coords, 0);
        glEnableVertexArrayAttrib(vao_, attribute_location_texture_coords);

        glCreateBuffers(1, &vbo_);
        glNamedBufferData(vbo_, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));

        count_ = static_cast<GLsizei>(vertices.size());
    }

    Mesh(std::vector<Vertex> const& vertices, std::vector<GLuint> const& indices, GLenum primitive_type) :
        Mesh{ vertices, primitive_type }
    {
        glCreateBuffers(1, &ebo_);
        glNamedBufferData(ebo_, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(vao_, ebo_);
        count_ = static_cast<GLsizei>(indices.size());
    }

    void draw() {
        glBindVertexArray(vao_);
        if (ebo_ == 0)
            glDrawArrays(primitive_type_, 0, count_);
        else
            glDrawElements(primitive_type_, count_, GL_UNSIGNED_INT, nullptr);
    }

    ~Mesh() {
        glDeleteBuffers(1, &ebo_);
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    };

private:
    GLenum primitive_type_{ GL_POINTS };
    GLsizei count_{ 0 };
    GLuint vao_{ 0 };
    GLuint vbo_{ 0 };
    GLuint ebo_{ 0 };
};
