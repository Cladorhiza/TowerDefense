#define GLEW_STATIC

#include "glew.h"
#include "gl/GL.h"
#include "glfw3.h"
#include "glm.hpp"

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>



#include "InputManager.h"
#include <Shader.h>
#include <stb_image.h>
#include <Texture.h>

const std::string SHADER_PATH {"res\\shaders\\"};
const std::string TEXTURE_PATH {"res\\textures\\"};

GLFWwindow* window;

glm::mat4 proj { glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f) };
glm::mat4 view { 1.0f };
glm::mat4 model{ 1.0f };

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

Vertex v1{ {-0.5f, -0.5f, 0.f},{1.f, 1.f, 1.f, 1.f}, {0.f, 0.f} };
Vertex v2{ {0.5f, -0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 0.f} };
Vertex v3{ {0.5f, 0.5f, 0.f},  {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f} };
Vertex v4{ {-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.f} };

const std::vector<Vertex> rectVerts{
    v1,
    v2,
    v3,
    v4
};



struct VertexArray {

    uint32_t vbo, vao, ibo;
    int indexCount;

    VertexArray(const std::vector<Vertex>& vertices) {

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
        indices.reserve(vertices.size() * 1.5f);
        for (int i{ 0 }; i+3 < vertices.size(); i += 4) {
            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(2);
            indices.push_back(0);
            indices.push_back(2);
            indices.push_back(3);
        }

        indexCount = indices.size();

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

    }

    void Bind() {
        glBindVertexArray(vao);
    }
    void UnBind() {
        glBindVertexArray(0);
    }
    ~VertexArray() {

        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);

    }

};

struct SpriteComponent {

    Texture texture;

    SpriteComponent(std::string texturePath) 
    :texture(texturePath)
    {

    }
};

namespace SpriteSystem {

    std::unordered_map<uint32_t, SpriteComponent> spriteComponents;

    void AddSprite(uint32_t id, std::string texturePath) {
        spriteComponents.emplace(id, texturePath);
    }






}

int Setup() {
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 960, "Tower Defense", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    InputManager::Init(window);

    return 1;
}

int main()
{
    
    if (Setup() == -1) {
        return -1;
    }
    
    Shader basicShader(SHADER_PATH + "quad.shader");
    basicShader.Bind();
    basicShader.SetUniformMat4f("ProjectionMatrix", proj);
    basicShader.SetUniformMat4f("ViewMatrix",       view);
    basicShader.SetUniformMat4f("ModelMatrix",      model);

    VertexArray vao(rectVerts);

    Texture bruno{TEXTURE_PATH + "bruno.png" };
    bruno.Bind();

    //Window Loop
    while (!glfwWindowShouldClose(window))
    {
        
        //INPUT
        InputManager::Poll(window);

        //LOGIC

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }



        //RENDER
        glClear(GL_COLOR_BUFFER_BIT);

        vao.Bind();
        glDrawElements(GL_TRIANGLES, vao.indexCount, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}