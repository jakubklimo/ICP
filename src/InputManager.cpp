#include "InputManager.h"
#include "ShaderLoaderApp.h"
#include <iostream>

InputManager::InputManager(GLFWwindow* win)
    : window(win), vsync_on(true)
{
    glfwSetKeyCallback(window,
        [](GLFWwindow* win, int key, int scancode, int action, int mods)
        {
            // zavoláme statickou InputManager callback
            InputManager::keyCallback(win, key, scancode, action, mods);
        });

    glfwSwapInterval(1);
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

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    ShaderLoaderApp* app = reinterpret_cast<ShaderLoaderApp*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    InputManager* manager = app->getInputManager();
    if (!manager) return;

    switch (key) {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;

    case GLFW_KEY_V:
        manager->toggleVSync();
        break;
    }
}


void InputManager::processInput() {
    
}
