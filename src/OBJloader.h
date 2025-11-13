#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "assets.h"

namespace OBJloader {

    bool loadOBJ(const std::string& path,
        std::vector<Vertex>& out_vertices,
        std::vector<GLuint>& out_indices);

}
