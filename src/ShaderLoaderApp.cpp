#include "ShaderLoaderApp.h"
#include "gl_err_callback.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "OBJloader.h"

ShaderLoaderApp::ShaderLoaderApp() = default;
ShaderLoaderApp::~ShaderLoaderApp() { cleanup(); }

void ShaderLoaderApp::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    ShaderLoaderApp* app = reinterpret_cast<ShaderLoaderApp*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    app->handleMouse(xpos, ypos);
}

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

bool ShaderLoaderApp::initGLFW() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "Shader Loader Demo", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return false; }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwWindowHint(GLFW_SAMPLES, 4);

    return true;
}

bool ShaderLoaderApp::initGLEW() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW error: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    return true;
}

bool ShaderLoaderApp::init() {
    if (!initGLFW()) return false;
    if (!initGLEW()) return false;

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    inputManager = std::make_unique<InputManager>(window);
    glfwSetWindowUserPointer(window, this);

    try {
        faceProcessor = std::make_unique<FaceProcessor>(
            "./resources/haarcascade_frontalface_default.xml",
            "./resources/lock.jpg",
            "./resources/warning.jpg"
        );
        faceProcessor->startBackgroundDetection();
    }
    catch (const std::exception& e) {
        std::cerr << "FaceProcessor init failed: " << e.what() << std::endl;
        return false;
    }

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

    initModel();
    return true;
}

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

        std::vector<Vertex> cubeVertices = {
            {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {0.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {0,0,1}, {1.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {0.0f, 1.0f}},
            {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {1.0f, 1.0f}},
            {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0.0f, 0.0f}},
            {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {0.0f, 1.0f}},
            {{ 0.5f,-0.5f,-0.5f}, {1,0,0}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {0.0f, 1.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {0,1,0}, {1.0f, 0.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {0.0f, 1.0f}},
            {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {0.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}, {1.0f, 1.0f}},
            {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {0.0f, 1.0f}},
        };

        std::vector<GLuint> cubeIndices = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
        };

        shaderCube = std::make_shared<ShaderProgram>("./shaders/textured.vert", "./shaders/textured.frag");
        auto cubeMesh = std::make_shared<Mesh>(cubeVertices, cubeIndices, GL_TRIANGLES);
        cubeTexture = std::make_unique<Texture>("./resources/box_rgb888.png");
        cubeModel = std::make_unique<Model>();
        cubeModel->addMesh(cubeMesh, shaderCube);

    }
    catch (const std::exception& e) {
        std::cerr << "Model load error: " << e.what() << std::endl;
    }
}

void ShaderLoaderApp::drawLoop() {
    float lastFrame = 0.0f;
    float globalTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {

        bool isFacePresent = faceProcessor->isFaceDetected();

        fps_meter.update();
        if (fps_meter.is_updated()) {
            current_fps = fps_meter.get();
            std::string title = "Shader Loader Demo - FPS: "
                + std::to_string((int)current_fps)
                + " - VSync: "
                + (inputManager->isVSync() ? "ON" : "OFF");
            glfwSetWindowTitle(window, title.c_str());
        }

        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        inputManager->processInput();

        if (isFacePresent) {
            processInput(deltaTime);
            globalTime += deltaTime;
        }

        if (inputManager->vsyncToggleRequested) {
            glfwSwapInterval(inputManager->isVSync() ? 1 : 0);
            inputManager->vsyncToggleRequested = false;
        }

        if (isFacePresent) {
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        }
        else {
            glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (model_) {
            shaderProgram->use();

            float t = globalTime;
            shaderProgram->setUniform("uColor",
                glm::vec3(
                    sin(t) * 0.5f + 0.5f,
                    0.5f,
                    cos(t) * 0.5f + 0.5f
                )
            );

            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
            glm::mat4 proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f,
                0.1f, 100.0f);

            shaderProgram->setUniform("uModel", model);
            shaderProgram->setUniform("uMVP", proj * view * model);

            model_->draw();
        }

        if (cubeModel) {
            shaderCube->use();
            cubeTexture->bind();

            float t = globalTime;
            glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
            glm::mat4 proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

            // První krychle (pùvodní)
            glm::mat4 model1 = glm::mat4(1.0f);
            model1 = glm::rotate(model1, t, glm::vec3(0.5f, 1.0f, 0.0f));

            glm::mat4 mvp1 = proj * view * model1;
            shaderCube->setUniform("uMVP", mvp1);
            cubeModel->draw();

            // Druhá krychle (posunutá o 2.5 jednotky doprava)
            glm::mat4 model2 = glm::mat4(1.0f);
            model2 = glm::translate(model2, glm::vec3(2.5f, 0.0f, 0.0f));
            model2 = glm::rotate(model2, t, glm::vec3(0.5f, 1.0f, 0.0f));

            glm::mat4 mvp2 = proj * view * model2;
            shaderCube->setUniform("uMVP", mvp2);
            cubeModel->draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void ShaderLoaderApp::run() {
    drawLoop();
}

void ShaderLoaderApp::cleanup() {
    if (faceProcessor) {
        faceProcessor->stopBackgroundDetection();
    }
    model_.reset();
    shaderProgram.reset();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

void ShaderLoaderApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ShaderLoaderApp* app = reinterpret_cast<ShaderLoaderApp*>(glfwGetWindowUserPointer(window));
    if (!app) return;
    app->handleScroll(yoffset);
}

void ShaderLoaderApp::handleScroll(double yoffset) {
    fov -= (float)yoffset;
    if (fov < 10.0f) fov = 10.0f;
    if (fov > 80.0f) fov = 80.0f;
}