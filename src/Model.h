#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 

#include <GL/glew.h>
#include <glm/glm.hpp> 

#include "assets.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "OBJloader.h"

class Model {
public:
    glm::vec3 pivot_position{};
    glm::vec3 eulerAngles{};
    glm::vec3 scale{ 1.0f };

    struct mesh_package {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<ShaderProgram> shader;
        glm::vec3 origin;
        glm::vec3 eulerAngles;
        glm::vec3 scale;

        mesh_package(std::shared_ptr<Mesh> m,
            std::shared_ptr<ShaderProgram> s,
            glm::vec3 o,
            glm::vec3 e,
            glm::vec3 sc)
            : mesh(m), shader(s), origin(o), eulerAngles(e), scale(sc) {
        }
    };

    std::vector<mesh_package> meshes;

    Model() = default;

    // Konstruktor: načte OBJ, vytvoří mesh
    Model(const std::filesystem::path& filename, std::shared_ptr<ShaderProgram> shader) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        if (!OBJloader::loadOBJ(filename.string(), vertices, indices)) {
            throw std::runtime_error("Failed to load OBJ file: " + filename.string());
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
        meshes.emplace_back(mesh, shader, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    }

    void addMesh(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<ShaderProgram> shader,
        glm::vec3 origin = glm::vec3(0.0f),
        glm::vec3 eulerAngles = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f))
    {
        meshes.emplace_back(mesh, shader, origin, eulerAngles, scale);
    }

    void update(float delta_t) {
        // volitelné pro animaci
    }

    void draw() {
        for (auto const& pkg : meshes) {
            pkg.shader->use();
            pkg.mesh->draw();
        }
    }
};
