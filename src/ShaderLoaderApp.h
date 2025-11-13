#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "ShaderProgram.h"
#include "Model.h"
#include <glm/glm.hpp>

class ShaderLoaderApp {
public:
    ShaderLoaderApp();
    ~ShaderLoaderApp();

    bool init();
    void run();

private:
    GLFWwindow* window = nullptr;
    std::shared_ptr<ShaderProgram> shaderProgram;
    std::unique_ptr<Model> model_;

    bool initGLFW();
    bool initGLEW();
    void initModel();
    void cleanup();
    void drawLoop();

    void processInput(float deltaTime);
    void handleMouse(double xpos, double ypos);

    glm::vec3 camPos{ 0.0f, 0.2f, 4.0f };
    glm::vec3 camFront{ 0.0f, 0.0f, -1.0f };
    glm::vec3 camUp{ 0.0f, 1.0f, 0.0f };

    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 400.0f, lastY = 300.0f;
    bool firstMouse = true;

    float movementSpeed = 8.5f;
    float mouseSensitivity = 0.1f;

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};
