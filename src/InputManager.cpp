#include "InputManager.h"
#include <iostream>

InputManager::InputManager(GLFWwindow* win)
    : window(win), vsync_on(true)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);
    updateWindowTitle();
}

void InputManager::toggleVSync() {
    vsync_on = !vsync_on;
    glfwSwapInterval(vsync_on ? 1 : 0);
    updateWindowTitle();
}

void InputManager::updateWindowTitle() {
    std::string title = "3D Triangle - VSync: ";
    title += (vsync_on ? "ON" : "OFF");
    glfwSetWindowTitle(window, title.c_str());
}

// static callback
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        InputManager* manager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            manager->toggleVSync();
            break;
        default:
            break;
        }
    }
}

void InputManager::processInput() {
    
}
