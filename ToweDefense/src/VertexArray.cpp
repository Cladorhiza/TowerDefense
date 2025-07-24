#include "VertexArray.h"

VertexArray::VertexArray(const std::vector<Vertex>& vertices) {

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        //shader layout location for vertexes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(0);

        //colours
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        //texcoords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 7));
        glEnableVertexAttribArray(2);

        std::vector<unsigned> indices;
        indices.reserve(static_cast<size_t>(vertices.size() * 1.5f));
        for (int i{ 0 }; i+3 < vertices.size(); i += 4) {
            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(2);
            indices.push_back(0);
            indices.push_back(2);
            indices.push_back(3);
        }

        indexCount = static_cast<int>(indices.size());

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

    }

    void VertexArray::Bind() {
        glBindVertexArray(vao);
    }
    void VertexArray::UnBind() {
        glBindVertexArray(0);
    }
    VertexArray::~VertexArray() {

        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);

    }