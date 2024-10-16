#pragma once
#include "glm.hpp"

struct Vertex {

    glm::vec3 pos;
    glm::vec4 color;
    glm::vec2 texCoord;

    Vertex()
        :pos(0.f, 0.f, 0.f), color(1.0f, 1.0f, 1.0f, 1.0f), texCoord(0.f, 0.f)
    {

    }

    Vertex(glm::vec3 pos, glm::vec4 color, glm::vec2 texCoord)
        :pos(pos), color(color), texCoord(texCoord)
    {

    }

};
