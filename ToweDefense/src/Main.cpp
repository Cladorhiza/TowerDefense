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
#include "Shader.h"
#include "stb_image.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Vertex.h"
#include "Rendering.h"
#include "Camera.h"

const std::string SHADER_PATH {"res\\shaders\\"};
const std::string TEXTURE_PATH {"res\\textures\\"};

GLFWwindow* window;

Camera camera;

glm::mat4 proj { glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f) };
glm::mat4 view { camera.GetViewMatrix() };
glm::mat4 model{ 1.0f };



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
        view = camera.GetViewMatrix();
        basicShader.SetUniformMat4f("VierMatrix", view);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }



        //RENDER
        glClear(GL_COLOR_BUFFER_BIT);

        Rendering::RenderSpriteVA(vao);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}