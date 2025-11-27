cmake_minimum_required(VERSION 3.30)

project(icp VERSION 2025.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# --- Balíčky přes vcpkg ---
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

# --- Seznam zdrojů ---
set(PROJECT_SOURCES
    src/main.cpp
    src/ImageProcessor.cpp
    src/CrossDrawer.cpp
    src/FaceProcessor.cpp
    src/fps_meter.cpp
    src/Renderer3D.cpp
    src/gl_err_callback.cpp

    src/ShaderLoaderApp.cpp
    src/ShaderProgram.cpp
    src/Texture.cpp
    src/OBJloader.cpp

    src/InputManager.cpp

    src/Texture.h
    src/Mesh.h
    src/Model.h
    src/ShaderProgram.h
    src/ShaderLoaderApp.h
    src/OBJloader.h
    src/assets.h
    src/non_copyable.h

    # ImGui
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} "src/Texture.cpp" "src/Texture.h")

target_link_libraries(${PROJECT_NAME} PRIVATE
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)
