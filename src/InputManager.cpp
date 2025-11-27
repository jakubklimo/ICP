#include "InputManager.h"
#include "ShaderLoaderApp.h"
#include <iostream>

InputManager::InputManager(GLFWwindow* win)
    : window(win), vsync_on(true)
{
    // statický callback, získá InputManager z user pointeru
    glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        InputManager* manager = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(win));
        if (manager)
            manager->keyCallback(win, key, scancode, action, mods);
        });

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
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    InputManager* manager = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!manager) return;

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

void InputManager::processInput() {
    // zde mùžeš pøípadnì implementovat další zpracování kláves
}
