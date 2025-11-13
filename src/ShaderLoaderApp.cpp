#include "ShaderLoaderApp.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ShaderLoaderApp::ShaderLoaderApp() = default;
ShaderLoaderApp::~ShaderLoaderApp() { cleanup(); }

bool ShaderLoaderApp::initGLFW() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!\n";
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Shader Loader Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    return true;
}

bool ShaderLoaderApp::initGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    return true;
}

bool ShaderLoaderApp::init() {
    if (!initGLFW()) return false;
    if (!initGLEW()) return false;

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);

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

    initTriangle();
    return true;
}

void ShaderLoaderApp::initTriangle() {
    const GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
}

void ShaderLoaderApp::drawLoop() {
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram->use();

        float t = static_cast<float>(glfwGetTime());
        shaderProgram->setUniform("uColor", glm::vec3(sin(t) * 0.5f + 0.5f, 0.5f, cos(t) * 0.5f + 0.5f));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void ShaderLoaderApp::run() {
    drawLoop();
}

void ShaderLoaderApp::cleanup() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}
