#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class InputManager {
public:
    InputManager(GLFWwindow* win);

    void updateWindowTitle();
    void processInput();
    bool isVSync() const { return vsync_on; }
    bool vsyncToggleRequested = false;

private:
    GLFWwindow* window;
    bool vsync_on;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void toggleVSync();
};