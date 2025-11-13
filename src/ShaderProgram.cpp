#include "ShaderProgram.h"
#include <fstream>
#include <sstream>
#include <iostream>

ShaderProgram::ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertSrc = readFile(vertexPath);
    std::string fragSrc = readFile(fragmentPath);

    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    programID = glCreateProgram();
    glAttachShader(programID, vertShader);
    glAttachShader(programID, fragShader);
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        throw std::runtime_error("Shader link error:\n" + std::string(infoLog));
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(programID);
}

void ShaderProgram::use() const {
    glUseProgram(programID);
}

std::string ShaderProgram::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot open shader file: " + path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        throw std::runtime_error("Shader compile error:\n" + std::string(infoLog));
    }

    return shader;
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    glUniform3fv(loc, 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& name, float value) const {
    GLint loc = glGetUniformLocation(programID, name.c_str());
    glUniform1f(loc, value);
}
