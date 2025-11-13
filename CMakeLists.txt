cmake_minimum_required(VERSION 3.30)

project(icp VERSION 2025.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# --- Balíèky pøes vcpkg ---
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

# --- Seznam zdrojù ---
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

    # ImGui
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
)

# --- Spustitelný soubor ---
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} "src/ShaderLoaderApp.h" "src/ShaderLoaderApp.cpp" "src/ShaderProgram.h")

# --- Linkování knihoven ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}
)

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
