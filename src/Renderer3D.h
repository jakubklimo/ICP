#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

// ImGui (backends z vcpkg)
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


class Renderer3D {
public:
    Renderer3D();
    bool init();
    void run();
    void cleanup();

private:
    GLFWwindow* window = nullptr;
    bool vsync_on = true;
    int fps = 0;
    bool cursor_captured = true;
    bool fullscreen = false;
    int windowed_x = 100, windowed_y = 100, windowed_width = 800, windowed_height = 600;
    void toggleFullscreen();

    bool initGLFW();
    bool initGLEW();
    void printSystemInfo();
    void drawTriangle();
    void updateWindowTitle();

    // Callbacks
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    // ImGui helpers
    void initImGui();
    void renderImGui();
    void shutdownImGui();
};
