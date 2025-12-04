#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 
#include <thread>

#include <GL/glew.h>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>

#include "assets.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "OBJloader.h"
#include "Texture.h" // Zajištění, že je Texture dostupná

class Model {
public:
    glm::vec3 pivot_position{};
    glm::vec3 eulerAngles{};
    glm::vec3 scale{ 1.0f };

    struct mesh_package {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<ShaderProgram> shader;
        // PŘIDÁNO: Pointer na texturu
        std::shared_ptr<Texture> texture;
        glm::vec3 origin;
        glm::vec3 eulerAngles;
        glm::vec3 scale;

        mesh_package(std::shared_ptr<Mesh> m,
            std::shared_ptr<ShaderProgram> s,
            std::shared_ptr<Texture> t, // S texturou
            glm::vec3 o,
            glm::vec3 e,
            glm::vec3 sc)
            : mesh(m), shader(s), texture(t), origin(o), eulerAngles(e), scale(sc) {
        }
        // Přetížení pro původní kód (bez textury)
        mesh_package(std::shared_ptr<Mesh> m,
            std::shared_ptr<ShaderProgram> s,
            glm::vec3 o,
            glm::vec3 e,
            glm::vec3 sc)
            : mesh(m), shader(s), texture(nullptr), origin(o), eulerAngles(e), scale(sc) {
        }
    };

    std::vector<mesh_package> meshes;

    Model() = default;

    // Konstruktor: načte OBJ, vytvoří mesh (použije se nullptr pro texturu)
    Model(const std::filesystem::path& filename, std::shared_ptr<ShaderProgram> shader) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        if (!OBJloader::loadOBJ(filename.string(), vertices, indices)) {
            throw std::runtime_error("Failed to load OBJ file: " + filename.string());
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
        meshes.emplace_back(mesh, shader, nullptr, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    }

    // Přetížení pro přidání s texturou
    void addMesh(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<ShaderProgram> shader,
        std::shared_ptr<Texture> texture, // S texturou
        glm::vec3 origin = glm::vec3(0.0f),
        glm::vec3 eulerAngles = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f))
    {
        meshes.emplace_back(mesh, shader, texture, origin, eulerAngles, scale);
    }

    // Přetížení pro původní kód (bez textury)
    void addMesh(std::shared_ptr<Mesh> mesh,
        std::shared_ptr<ShaderProgram> shader,
        glm::vec3 origin = glm::vec3(0.0f),
        glm::vec3 eulerAngles = glm::vec3(0.0f),
        glm::vec3 scale = glm::vec3(1.0f))
    {
        meshes.emplace_back(mesh, shader, nullptr, origin, eulerAngles, scale);
    }


    void update(float delta_t) {
        // volitelné pro animaci
    }

    void draw() {
        for (auto const& pkg : meshes) {
            pkg.shader->use();
            if (pkg.texture) {
                // Aktivujeme texturu (předpokládáme slot 0)
                pkg.texture->bind();
                // Nastavíme uniform sampler2D na slot 0
                // Opraveno: Používáme id() pro získání ID programu
                glUniform1i(glGetUniformLocation(pkg.shader->id(), "uTexture"), 0);
            }
            pkg.mesh->draw();
            if (pkg.texture) {
                pkg.texture->unbind();
            }
        }
    }
};