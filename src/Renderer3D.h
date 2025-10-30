#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Renderer3D {
public:
    Renderer3D();
    bool init();
    void run();
    void cleanup();

private:
    GLFWwindow* window = nullptr;

    bool initGLFW();
    bool initGLEW();
    void printSystemInfo();
    void drawTriangle();
};
