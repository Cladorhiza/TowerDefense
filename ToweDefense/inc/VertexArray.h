#pragma once
#include "glew.h"
#include "gl/GL.h"

#include "Vertex.h"

#include <vector>
struct VertexArray {

    int indexCount;

    VertexArray(const std::vector<Vertex>& vertices);
    ~VertexArray();

    void Bind();
    void UnBind();

private:

    uint32_t vbo, vao, ibo;
    
    VertexArray(const VertexArray&) = delete;
    VertexArray(VertexArray&&) = delete;

};
