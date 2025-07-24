#define GLEW_STATIC

#include "glew.h"
#include "gl/GL.h"
#include "glfw3.h"
#include "glm.hpp"

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <utility>

#include "InputManager.h"
#include "Shader.h"
#include "stb_image.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Vertex.h"
#include "Rendering.h"
#include "Camera.h"

#define COCK std::cout << "cock\n";
#define PVEC2(vec) std::cout  << vec.x << ", " << vec.y << '\n';
#define PVEC3(vec) std::cout  << vec.x << ", " << vec.y << ", " << vec.z << '\n';

const std::string SHADER_PATH {"res\\shaders\\"};
const std::string TEXTURE_PATH {"res\\textures\\"};

GLFWwindow* window;

Camera camera;

glm::mat4 proj { glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f) };
glm::mat4 view { camera.GetViewMatrix() };
glm::mat4 model { 1.0f };

double deltaTime { 0.f };



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

struct TransformComponent{
    
    TransformComponent()
        :position(0.0f), eulerRotation(0.0f), scale(1.0f), transform(1.0f), modified(false)
    {
    
    }

    glm::vec3 position;
    glm::vec3 eulerRotation;
    glm::vec3 scale;
    glm::mat4 transform;
    bool modified;
};

namespace TransformSystem{

    std::unordered_map<uint32_t, TransformComponent> transforms;

    void AddTransform(uint32_t id){
        bool success = transforms.emplace(std::piecewise_construct, std::make_tuple(id), std::make_tuple()).second;
        if (!success) std::cout << "TransformSystem: failed to insert transform component " << id << " into system!\n";
    }

    glm::mat4 GetTransform(uint32_t id) {

        TransformComponent tc{ transforms[id] };

        //recalculate if transformcomponent has been modified since last get
        if (tc.modified) {
            glm::mat4 t{ 1.0f };
            t = glm::translate(t, tc.position);
            t = glm::rotate(t, tc.eulerRotation.x, { 1.0f, 0.0f, 0.0f });
            t = glm::rotate(t, tc.eulerRotation.y, { 0.0f, 1.0f, 0.0f });
            t = glm::rotate(t, tc.eulerRotation.z, { 0.0f, 0.0f, 1.0f });
            transforms[id].transform = glm::scale(t, tc.scale);
        }

        return transforms[id].transform;

    }

    TransformComponent GetComponent(uint32_t id){
        if (transforms.find(id) != transforms.end()){
            return transforms.at(id);
        }
        else {
            std::cout << "TransformSystem: failed to retrieve transform component " << id << " from system!\n";
            return TransformComponent{};
        }
    }

    void SetTransform(uint32_t id, TransformComponent newTransform){
        transforms[id] = newTransform;
        transforms[id].modified = true;
    }
}

struct SpriteComponent {

    VertexArray vao;
    Texture texture;

    SpriteComponent(std::string texturePath, std::vector<Vertex> vertices) 
    :texture(texturePath), vao(vertices)
    {

    }
};

namespace SpriteSystem {

    std::unordered_map<uint32_t, SpriteComponent> sprites;
    Shader* shader;

    void AddSprite(uint32_t id, std::string texturePath, std::vector<Vertex> vertices) {
        bool success = sprites.emplace(std::piecewise_construct, std::make_tuple(id), std::make_tuple(texturePath, vertices)).second;
        if (!success) std::cout << "SpriteSystem: failed to insert sprite component " << id << " into system!\n";
    }
    
    void Update(){
        if (!shader) {
            std::cout << "SpriteSystem: failed to render without bound shader!\n";
            return;
        }
        shader->Bind();

        for (auto& [id,sprite] : sprites) {
            
            shader->SetUniformMat4f("ModelMatrix", TransformSystem::GetTransform(id));
            sprite.texture.Bind();
            Rendering::RenderSpriteVA(sprite.vao);
            
            
        }
        
        shader->Unbind();
    }

    void SetShader(Shader& s){
        shader = &s;
    }
}

struct WaypointMovementComponent {

    std::vector<glm::vec2> waypoints;
    size_t currentWaypoint;
    float speed;

    WaypointMovementComponent()
        :waypoints{}, currentWaypoint{ 0 }, speed{ 0.f }
    {

    }

};

namespace WaypointMovementSystem {

    std::unordered_map<uint32_t, WaypointMovementComponent> components;

    void AddComponent(uint32_t id) {
        components.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(WaypointMovementComponent{}));
    }
    void AddComponent(uint32_t id, WaypointMovementComponent wmc) {
        components.emplace(id, wmc);
    }

    WaypointMovementComponent& GetComponent(uint32_t id) {
        return components[id];
    }

    void Update() {

        for (auto& [id, component] : components) {
            TransformComponent tc { TransformSystem::GetComponent(id) };
            glm::vec2 waypointPos { component.waypoints[component.currentWaypoint] };
            glm::vec2 direction { waypointPos.x - tc.position.x, waypointPos.y - tc.position.y };
            //if difference between position and waypoint is small enough, change current waypoint to next
            if (glm::length(direction) < 0.01f) {
                if (component.currentWaypoint < component.waypoints.size() - 1) {
                    component.currentWaypoint++;
                    //TODO: this wastes a tick, consider recalculating new movement
                    continue;
                }
            }
            direction = glm::normalize(direction) * component.speed;
            tc.position.x += direction.x;
            tc.position.y += direction.y;
            TransformSystem::SetTransform(id, tc);
        }

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

    SpriteSystem::SetShader(basicShader);

    //bruno waypoints
    TransformSystem::AddTransform(0);
    SpriteSystem::AddSprite(0, TEXTURE_PATH + "bruno.png", rectVerts);
    WaypointMovementComponent brunoWaypoints;
    brunoWaypoints.currentWaypoint = 0;
    brunoWaypoints.waypoints.emplace_back(-9.0f, 9.0f);
    brunoWaypoints.waypoints.emplace_back( 9.0f, 9.0f);
    brunoWaypoints.waypoints.emplace_back( 9.0f,-9.0f);
    brunoWaypoints.waypoints.emplace_back(-9.0f,-9.0f);
    brunoWaypoints.speed = 0.01f;
    WaypointMovementSystem::AddComponent(0, brunoWaypoints);

    //Window Loop
    while (!glfwWindowShouldClose(window))
    {
        
        //INPUT
        InputManager::Poll(window);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        //LOGIC
        
        //system updates


        //camera
        view = camera.GetViewMatrix();
        basicShader.SetUniformMat4f("ViewMatrix", view);

        //waypoint system
        WaypointMovementSystem::Update();


        //RENDER
        glClear(GL_COLOR_BUFFER_BIT);

        //rendering system updates
        SpriteSystem::Update();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}