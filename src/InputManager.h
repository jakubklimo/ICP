#pragma once
#include <GLFW/glfw3.h>
#include <string>

class InputManager {
public:
    InputManager(GLFWwindow* win);

    void processInput();
    void updateWindowTitle();

    bool isVSync() const { return vsync_on; }

private:
    GLFWwindow* window;
    bool vsync_on;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void toggleVSync();
};