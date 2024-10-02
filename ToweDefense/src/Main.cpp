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

const std::string shaderPath {"res\\shaders\\"};

GLFWwindow* window;

glm::mat4 proj { glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f) };
glm::mat4 view { 1.0f };
glm::mat4 model{ 1.0f };

struct Vertex {

    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    Vertex()
        :pos(0.f, 0.f, 0.f), color(1.0f, 1.0f, 1.0f), texCoord(0.f, 0.f)
    {

    }

    Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 texCoord)
        :pos(pos), color(color), texCoord(texCoord)
    {

    }

};

Vertex v1{ glm::vec3(-0.5f, -0.5f, 0.f),glm::vec3(1.f, 1.f, 1.f), glm::vec2(0.f, 0.f) };
Vertex v2{ glm::vec3(0.5f, -0.5f, 0.f),glm::vec3(1.f, 1.f, 1.f), glm::vec2(1.f, 0.f) };
Vertex v3{ glm::vec3(0.5f, 0.5f, 0.f),glm::vec3(1.f, 1.f, 1.f), glm::vec2(1.f, 1.f) };
Vertex v4{ glm::vec3(-0.5f, 0.5f, 0.f),glm::vec3(1.f, 1.f, 1.f), glm::vec2(0.f, 1.f) };

const std::vector<Vertex> rectVerts{
    v1,
    v2,
    v3,
    v4
};

const std::vector<unsigned> rectIndices{
    0,
    1,
    2,
    0,
    2,
    3
};

struct SpriteComponent {

    std::string texturePath;



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
    
    Shader basicShader(shaderPath + "quad.shader");
    basicShader.Bind();
    basicShader.SetUniformMat4f("ProjectionMatrix", proj);
    basicShader.SetUniformMat4f("ViewMatrix",       view);
    basicShader.SetUniformMat4f("ModelMatrix",      model);

    unsigned vbo, ibo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, rectVerts.size() * sizeof(Vertex), rectVerts.data(), GL_STATIC_DRAW);
    
    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    //colours
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    //texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), rectIndices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    int width, height, bpp;

    stbi_set_flip_vertically_on_load(1);
    unsigned char* texBuffer = stbi_load("res\\textures\\bruno.png", &width, &height, &bpp, 4);
    //if (!texBuffer) std::cout << "Texture at path: " << filePath << " failed to load!" << std::endl;

    unsigned id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer);
    //glBindTexture(GL_TEXTURE_2D, 0);

    if (texBuffer)
        stbi_image_free(texBuffer);


    //Window Loop
    while (!glfwWindowShouldClose(window))
    {
        
        //INPUT
        InputManager::Poll(window);



        //LOGIC




        //RENDER
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}