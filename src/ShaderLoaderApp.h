#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "ShaderProgram.h"

class ShaderLoaderApp {
public:
    ShaderLoaderApp();
    ~ShaderLoaderApp();

    bool init();
    void run();

private:
    GLFWwindow* window = nullptr;
    std::shared_ptr<ShaderProgram> shaderProgram;

    GLuint VAO = 0, VBO = 0;

    bool initGLFW();
    bool initGLEW();
    void initTriangle();
    void cleanup();

    void drawLoop();
};
