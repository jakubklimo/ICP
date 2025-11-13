#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <stdexcept>

class ShaderProgram {
public:
    ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
    ~ShaderProgram();

    void use() const;
    GLuint id() const { return programID; }

    void setUniform(const std::string& name, const glm::vec3& value) const;
    void setUniform(const std::string& name, float value) const;
    void setUniform(const std::string& name, const glm::mat4& value) const;

private:
    GLuint programID;

    std::string readFile(const std::string& path);
    GLuint compileShader(GLenum type, const std::string& source);
};
