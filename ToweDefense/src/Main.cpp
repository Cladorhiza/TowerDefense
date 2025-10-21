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
#include <chrono>
#include <random>

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

const int GAME_TICKRATE { 128 };
constexpr double GAME_TIME_PER_TICK = 1.0 / GAME_TICKRATE;

const uint32_t TIME_BETWEEN_WAVES{ 15 };

GLFWwindow* window;

Camera camera;

glm::mat4 proj { glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f) };
glm::mat4 view { camera.GetViewMatrix() };
glm::mat4 model { 1.0f };

double deltaTime { 0.f };

const std::vector<Vertex> rectVerts{
    { {-0.5f, -0.5f, 0.f},{1.f, 1.f, 1.f, 1.f}, {0.f, 0.f} },
    { {0.5f, -0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {1.f, 0.f} },
    { {0.5f, 0.5f, 0.f},  {1.f, 1.f, 1.f, 1.f}, {1.f, 1.f} },
    { {-0.5f, 0.5f, 0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 1.f} }
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

struct FrameTimer {
public:

    int32_t frameCount;
    double deltaTime;

private:

    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    std::chrono::duration<float> elapsed;
    double cumulativeDelta;

public:

    FrameTimer()
        :frameCount(0), deltaTime(0.0), cumulativeDelta(0.0), elapsed(), start(), end()
    {

    }

    void FrameStart() {
        start = std::chrono::high_resolution_clock::now();
    }

    void FrameEnd() {
        end = std::chrono::high_resolution_clock::now();
        elapsed = end - start;
        deltaTime = elapsed.count();
        cumulativeDelta += deltaTime;

        frameCount++;
        if (cumulativeDelta > std::chrono::seconds(1).count()) {
            cumulativeDelta -= std::chrono::seconds(1).count();
            std::cout << frameCount << std::endl;
            frameCount = 0;
        }
    }

};

struct WaveEnemyComponent {

    WaveEnemyComponent(int32_t bounty, double speed) 
        :bounty(bounty), speed(speed)
    {

    }
    //money gained on kill
    int32_t bounty;
    //movement speed
    double speed;

};

WaveEnemyComponent brunowave(10, 0.001);

namespace WaveSystem {

    std::vector<std::vector<std::pair<WaveEnemyComponent, uint32_t>>> waves;

    double currBetweenWaveTime{ 0.f };
    bool waveInProgress = false;


    void Update() {
        
        if (!waveInProgress) {
            waves.emplace_back();
            waves[0].emplace_back(std::piecewise_construct, std::forward_as_tuple(brunowave), std::forward_as_tuple(10));

            WaypointMovementComponent brunoWaypoints;
            brunoWaypoints.currentWaypoint = 0;
            brunoWaypoints.waypoints.emplace_back(-9.0f, 9.0f);
            brunoWaypoints.waypoints.emplace_back(9.0f, 9.0f);
            brunoWaypoints.waypoints.emplace_back(9.0f, -9.0f);
            brunoWaypoints.waypoints.emplace_back(-9.0f, -9.0f);

            for (int i{ 0 }; i < waves[0][0].second; i++) {
                TransformSystem::AddTransform(i);
                TransformComponent tc = TransformSystem::GetComponent(i);
                tc.position = glm::vec3(((std::rand() % 10) / 5.f) - 1, ((std::rand() % 10) / 5.f) - 1, 0.0f);
                TransformSystem::SetTransform(i, tc);
                SpriteSystem::AddSprite(i, TEXTURE_PATH + "bruno.png", rectVerts);
                
                brunoWaypoints.speed = waves[0][0].first.speed;
                WaypointMovementSystem::AddComponent(i, brunoWaypoints);
            }
            waveInProgress = true;
        }
        

    }



}

namespace PhysicsUtil {

    //aabb collision

    bool CirclesCollide(float x1, float x2, float y1, float y2, float r1, float r2) {
        float absDiffXY = ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2));
        return absDiffXY <= (r1 + r2);
    }
}

int Setup() {
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 960, "Tower Defense", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    //disable vsync
    glfwSwapInterval(0);

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
    
    FrameTimer ft;
    double gameTime{ 0.0f };

    Shader basicShader(SHADER_PATH + "quad.shader");
    basicShader.Bind();
    basicShader.SetUniformMat4f("ProjectionMatrix", proj);
    basicShader.SetUniformMat4f("ViewMatrix",       view);
    basicShader.SetUniformMat4f("ModelMatrix",      model);

    SpriteSystem::SetShader(basicShader);

    SpriteSystem::AddSprite(11, TEXTURE_PATH + "tower.png", rectVerts);
    TransformSystem::AddTransform(11);
    TransformComponent towerLocation;
    towerLocation.position = glm::vec3{ 3.0f, 3.0f, 1.0f };
    TransformSystem::SetTransform(11, towerLocation);
    
    //Window Loop
    while (!glfwWindowShouldClose(window))
    {
        ft.FrameStart();
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

        
        WaveSystem::Update();

        WaypointMovementSystem::Update();


        //RENDER
        glClear(GL_COLOR_BUFFER_BIT);

        //rendering system updates
        SpriteSystem::Update();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        ft.FrameEnd();
        gameTime += ft.deltaTime;
    }

    glfwTerminate();
    return 0;
}