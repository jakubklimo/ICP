#include "OBJloader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace OBJloader {

    bool loadOBJ(const std::string& path,
        std::vector<Vertex>& out_vertices,
        std::vector<GLuint>& out_indices)
    {
        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec2> temp_texcoords;
        std::vector<glm::vec3> temp_normals;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Cannot open OBJ file: " << path << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                iss >> pos.x >> pos.y >> pos.z;
                temp_positions.push_back(pos);
            }
            else if (prefix == "vt") {
                glm::vec2 tex;
                iss >> tex.x >> tex.y;
                temp_texcoords.push_back(tex);
            }
            else if (prefix == "vn") {
                glm::vec3 n;
                iss >> n.x >> n.y >> n.z;
                temp_normals.push_back(n);
            }
            else if (prefix == "f") {
                std::vector<int> vertex_indices;
                std::vector<int> tex_indices;
                std::vector<int> normal_indices;

                std::string token;
                while (iss >> token) {
                    std::replace(token.begin(), token.end(), '/', ' ');
                    std::istringstream vtn(token);
                    int v = 0, t = 0, n = 0;
                    vtn >> v >> t >> n;
                    vertex_indices.push_back(v);
                    tex_indices.push_back(t);
                    normal_indices.push_back(n);
                }

                // rozbij quady na dva trojúhelníky, nebo vykresli trojúhelník
                for (size_t i = 1; i + 1 < vertex_indices.size(); i++) {
                    int idx[3] = { 0, static_cast<int>(i), static_cast<int>(i + 1) };
                    for (int k = 0; k < 3; k++) {
                        Vertex vert{};

                        // position
                        if (vertex_indices[idx[k]] > 0 && vertex_indices[idx[k]] <= (int)temp_positions.size())
                            vert.position = temp_positions[vertex_indices[idx[k]] - 1];
                        else
                            vert.position = glm::vec3(0.0f);

                        // texCoords
                        if (!temp_texcoords.empty() && tex_indices[idx[k]] > 0 && tex_indices[idx[k]] <= (int)temp_texcoords.size())
                            vert.texCoords = temp_texcoords[tex_indices[idx[k]] - 1];
                        else
                            vert.texCoords = glm::vec2(0.0f);

                        // normal
                        if (!temp_normals.empty() && normal_indices[idx[k]] > 0 && normal_indices[idx[k]] <= (int)temp_normals.size())
                            vert.normal = temp_normals[normal_indices[idx[k]] - 1];
                        else {
                            // spoèítat normálu z trojúhelníku
                            glm::vec3 v0 = temp_positions[vertex_indices[idx[0]] - 1];
                            glm::vec3 v1 = temp_positions[vertex_indices[idx[1]] - 1];
                            glm::vec3 v2 = temp_positions[vertex_indices[idx[2]] - 1];
                            vert.normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
                        }

                        // pøidání vertexu a indexu
                        out_vertices.push_back(vert);
                        out_indices.push_back(static_cast<GLuint>(out_vertices.size() - 1));
                    }
                }
            }
        }

        std::cout << "Loaded OBJ: " << path << " (" << out_vertices.size() << " vertices, "
            << out_indices.size() << " indices)" << std::endl;

        return true;
    }

} // namespace OBJloader
