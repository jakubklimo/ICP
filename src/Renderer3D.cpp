#include "Renderer3D.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <thread>
#include <chrono>


namespace {
    float bg_r = 0.1f, bg_g = 0.1f, bg_b = 0.1f;
    double mouse_x = 0, mouse_y = 0;
    const double maxFPS = 60.0;
}

Renderer3D::Renderer3D() = default;

bool Renderer3D::init() {
    if (!initGLFW()) return false;
    if (!initGLEW()) return false;

    printSystemInfo();
    initImGui();

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return true;
}

bool Renderer3D::initGLFW() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "3D Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(vsync_on ? 1 : 0);

    glfwShowWindow(window);
    return true;
}

bool Renderer3D::initGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    return true;
}

void Renderer3D::printSystemInfo() {
    std::cout << "===== OpenGL System Info =====\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    std::cout << "=============================\n";
}

void Renderer3D::updateWindowTitle() {
    std::ostringstream title;
    title << "3D Triangle - FPS: " << fps << " - VSync: " << (vsync_on ? "ON" : "OFF");
    glfwSetWindowTitle(window, title.str().c_str());
}

// --- ImGui ---

void Renderer3D::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void Renderer3D::renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Renderer Controls");
    ImGui::Text("FPS: %d", fps);
    if (ImGui::Checkbox("VSync", &vsync_on)) {
        glfwSwapInterval(vsync_on ? 1 : 0);
    }
    if (ImGui::Button(cursor_captured ? "Release Cursor (TAB)" : "Capture Cursor (TAB)")) {
        cursor_captured = !cursor_captured;
        glfwSetInputMode(window, GLFW_CURSOR, cursor_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer3D::shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// --- Callbacks ---

void Renderer3D::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Renderer3D* renderer = static_cast<Renderer3D*>(glfwGetWindowUserPointer(window));
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            renderer->vsync_on = !renderer->vsync_on;
            glfwSwapInterval(renderer->vsync_on ? 1 : 0);
            break;
        case GLFW_KEY_TAB:
            renderer->cursor_captured = !renderer->cursor_captured;
            glfwSetInputMode(window, GLFW_CURSOR, renderer->cursor_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_Q:
            renderer->toggleFullscreen();
            break;
        default: break;
        }
    }
}

void Renderer3D::toggleFullscreen() {
    fullscreen = !fullscreen;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);

    if (fullscreen) {
        glfwGetWindowPos(window, &windowed_x, &windowed_y);
        glfwGetWindowSize(window, &windowed_width, &windowed_height);
        glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else {
        glfwSetWindowMonitor(window, nullptr, windowed_x, windowed_y, windowed_width, windowed_height, 0);
    }

    // Po pøepnutí fullscreen udrž VSync
    glfwSwapInterval(vsync_on ? 1 : 0);
}

void Renderer3D::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Renderer3D::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        bg_r = 0.5f; bg_g = 0.2f; bg_b = 0.8f;
    }
}

void Renderer3D::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_x = xpos; mouse_y = ypos;
    bg_g = float(xpos / 800.0);
}

void Renderer3D::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    bg_b += float(yoffset * 0.05);
    if (bg_b > 1.0f) bg_b = 1.0f;
    if (bg_b < 0.0f) bg_b = 0.0f;
}

// --- Render loop ---

void Renderer3D::drawTriangle() {
    const GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const char* vertexShaderSrc = R"(
        #version 460 core
        layout(location = 0) in vec3 position;
        void main() { gl_Position = vec4(position, 1.0); }
    )";

    const char* fragmentShaderSrc = R"(
        #version 460 core
        out vec4 FragColor;
        uniform vec4 triColor;
        void main() { FragColor = triColor; }
    )";

    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[512];
            glGetShaderInfoLog(shader, 512, nullptr, info);
            std::cerr << "Shader compile error: " << info << std::endl;
        }
        return shader;
        };

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    GLint uniform_color_location = glGetUniformLocation(shaderProgram, "triColor");

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window)) {
        double startTime = glfwGetTime();

        // --- FPS poèítání ---
        nbFrames++;
        if (startTime - lastTime >= 1.0) {
            fps = nbFrames;
            updateWindowTitle();
            nbFrames = 0;
            lastTime += 1.0;
        }

        // --- Render ---
        glClearColor(bg_r, bg_g, bg_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = float(glfwGetTime());
        float r = (sin(t) * 0.5f + 0.5f);
        float g = (cos(t) * 0.5f + 0.5f);
        float b = (sin(t * 0.3f) * 0.5f + 0.5f);
        glUseProgram(shaderProgram);
        glUniform4f(uniform_color_location, r, g, b, 1.0f);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        renderImGui();

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (vsync_on) {
            double frameTime = glfwGetTime() - startTime;
            double minFrameTime = 1.0 / (maxFPS*2);
            if (frameTime < minFrameTime) {
                std::this_thread::sleep_for(std::chrono::duration<double>(minFrameTime - frameTime));
            }
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Renderer3D::run() {
    drawTriangle();
    shutdownImGui();
    cleanup();
}

void Renderer3D::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
