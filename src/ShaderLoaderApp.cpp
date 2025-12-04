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

    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwWindowHint(GLFW_SAMPLES, 4);


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
    glEnable(GL_MULTISAMPLE);


    // zde VYTVOØ InputManager
    inputManager = std::make_unique<InputManager>(window);
    glfwSetWindowUserPointer(window, this);



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

        std::vector<Vertex> cubeVertices = {
            // front
            {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {0.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {0,0,1}, {1.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {0.0f, 1.0f}},

            // back
            {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {1.0f, 1.0f}},

            // left
            {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0.0f, 0.0f}},
            {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {0.0f, 1.0f}},

            // right
            {{ 0.5f,-0.5f,-0.5f}, {1,0,0}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {0.0f, 1.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {1.0f, 1.0f}},

            // top
            {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {0.0f, 0.0f}},
            {{ 0.5f, 0.5f, 0.5f}, {0,1,0}, {1.0f, 0.0f}},
            {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {0.0f, 1.0f}},

            // bottom
            {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {0.0f, 0.0f}},
            {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1.0f, 0.0f}},
            {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}, {1.0f, 1.0f}},
            {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {0.0f, 1.0f}},
        };

        // Indices
        std::vector<GLuint> cubeIndices = {
        0,1,2, 2,3,0,       // front
        4,5,6, 6,7,4,       // back
        8,9,10, 10,11,8,    // left
        12,13,14, 14,15,12, // right
        16,17,18, 18,19,16, // top
        20,21,22, 22,23,20  // bottom
        };





        // Naètení shaderu pro krychli
        shaderCube = std::make_shared<ShaderProgram>("./shaders/textured.vert", "./shaders/textured.frag");

        // Vytvoøení mesh krychle
        auto cubeMesh = std::make_shared<Mesh>(cubeVertices, cubeIndices, GL_TRIANGLES);

        // Textura
        cubeTexture = std::make_unique<Texture>("./resources/box_rgb888.png");

        // Model krychle
        cubeModel = std::make_unique<Model>();
        cubeModel->addMesh(cubeMesh, shaderCube);

    }
    catch (const std::exception& e) {
        std::cerr << "Model load error: " << e.what() << std::endl;
    }
}

// --- Hlavní vykreslovací smyèka ---
void ShaderLoaderApp::drawLoop() {
    float lastFrame = 0.0f;

        while (!glfwWindowShouldClose(window)) {

            // --- FPS ---
            fps_meter.update();
            if (fps_meter.is_updated()) {
                current_fps = fps_meter.get();

                std::string title = "Shader Loader Demo - FPS: "
                    + std::to_string((int)current_fps)
                    + " - VSync: "
                    + (inputManager->isVSync() ? "ON" : "OFF");

                glfwSetWindowTitle(window, title.c_str());
            }

            // èas
            float currentFrame = (float)glfwGetTime();
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // vstupy
            inputManager->processInput();
            processInput(deltaTime);

            // --- bezpeèné pøepnutí VSync ---
            if (inputManager->vsyncToggleRequested) {
                glfwSwapInterval(inputManager->isVSync() ? 1 : 0);
                inputManager->vsyncToggleRequested = false;
            }

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (model_) {
                shaderProgram->use();

                // --- nastavení uniform ---
                float t = (float)glfwGetTime();
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

                // --- vykreslení modelu ---
                model_->draw();
            }

            // krychle
            if (cubeModel) {
                shaderCube->use();

                // bind textury
                cubeTexture->bind();

                float t = (float)glfwGetTime();
                glm::mat4 model = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0.5f, 1.0f, 0.0f));
                glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
                glm::mat4 proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

                glm::mat4 mvp = proj * view * model;
                shaderCube->setUniform("uMVP", mvp);

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

