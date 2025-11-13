#include "ShaderLoaderApp.h"
#include "gl_err_callback.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "OBJloader.h"

ShaderLoaderApp::ShaderLoaderApp() = default;
ShaderLoaderApp::~ShaderLoaderApp() { cleanup(); }

// --- Statický callback pro myš ---
void ShaderLoaderApp::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    ShaderLoaderApp* app = reinterpret_cast<ShaderLoaderApp*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    app->handleMouse(xpos, ypos);
}

// --- Zpracování pohybu myší ---
void ShaderLoaderApp::handleMouse(double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos);
    lastX = (float)xpos;
    lastY = (float)ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);
}

// --- Pohyb kamery ---
void ShaderLoaderApp::processInput(float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camFront * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += glm::normalize(glm::cross(camFront, camUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos += camUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camPos -= camUp * velocity;
}

// --- Inicializace GLFW ---
bool ShaderLoaderApp::initGLFW() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Shader Loader Demo", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return false; }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouse_callback);

    return true;
}

// --- Inicializace GLEW ---
bool ShaderLoaderApp::initGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW error: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    return true;
}

// --- Inicializace aplikace ---
bool ShaderLoaderApp::init() {
    if (!initGLFW()) return false;
    if (!initGLEW()) return false;

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);

    // shader
    try {
        shaderProgram = std::make_shared<ShaderProgram>(
            "./shaders/simple.vert",
            "./shaders/simple.frag"
        );
    }
    catch (const std::exception& e) {
        std::cerr << "Shader load error: " << e.what() << std::endl;
        return false;
    }

    initModel(); // naètení modelu
    return true;
}

// --- Naètení modelu ---
void ShaderLoaderApp::initModel() {
    try {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        if (!OBJloader::loadOBJ("./resources/bunny_tri_vn.obj", vertices, indices)) {
            throw std::runtime_error("Failed to load bunny_tri_vn.obj");
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);

        model_ = std::make_unique<Model>();
        model_->addMesh(mesh, shaderProgram, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    }
    catch (const std::exception& e) {
        std::cerr << "Model load error: " << e.what() << std::endl;
    }
}

// --- Hlavní vykreslovací smyèka ---
void ShaderLoaderApp::drawLoop() {
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        processInput(deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (model_) {
            shaderProgram->use();

            float t = static_cast<float>(glfwGetTime());
            shaderProgram->setUniform("uColor",
                glm::vec3(sin(t) * 0.5f + 0.5f, 0.5f, cos(t) * 0.5f + 0.5f));

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            glm::mat4 mvp = proj * view * model;

            shaderProgram->setUniform("uMVP", mvp);

            model_->draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void ShaderLoaderApp::run() {
    drawLoop();
}

void ShaderLoaderApp::cleanup() {
    model_.reset();
    shaderProgram.reset();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}
