#ifndef car_controller_hpp
#define car_controller_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../vehicle/vehicle.h"
#include "../utils/bullet_glm.h"
#include "../particle_engine/particle_engine.h"
#include "../camera/camera.h"
#include "../render_manager/render_manager.h"
#include "../shader_manager/shader_manager.h"
#include "../transform_link/link_rigidbody.h"
#include "../update_manager/update_manager.h"

#include "link_wheel.h"
#include "link_door.h"

struct Follow
{
    glm::vec3 offset;
    float distance;
    float stretch;
    float stretchMax;
    float stretchTarget;
    float stretchSpeed;
    float stretchSteeringFactor;
    float yOffset;
    float yStretchFactor;
    float angularSpeed;
};

struct Models
{
    Model *carBody;
    Model *carHood;
    Model *carTrunk;
    Model *wheelBase;
    Model *doorFront;
    Model *doorRear;
    Model *smokeParticleModel;
};

class CarController : public Updatable
{
public:
    CarController(GLFWwindow *window,
                  ShaderManager *shaderManager,
                  RenderManager *renderManager,
                  ResourceManager *resourceManager,
                  Vehicle *vehicle,
                  Camera *followCamera,
                  Models models,
                  int exhaustCount);
    ~CarController();

    GLFWwindow *m_window;
    Vehicle *m_vehicle;
    Camera *m_followCamera;
    Models m_models;
    int m_exhaustCount;
    std::vector<ParticleEngine *> m_exhausParticles;
    std::vector<ParticleEngine *> m_tireSmokeParticles;
    Shader m_exhaustShader;
    Shader m_tireSmokeShader;

    glm::vec3 m_rotation = glm::vec3(0.f, -M_PI_2, 0.f);

    // enter-car pathfind
    glm::vec2 m_safeSize = glm::vec2(1.f, 3.f);
    glm::vec2 m_doorOffset = glm::vec2(2.17f, -0.86f);
    // enter-car animation
    glm::vec3 m_animDoorOffset = glm::vec3(2.46f, 0.210f, -0.85f);

    bool m_controlVehicle = false;
    bool m_followVehicle = false;

    glm::vec3 m_pos = glm::vec3(0.0f, 2.5f, 0.0f);
    Follow m_follow;

    RenderSource *m_bodySource;
    RenderSource *m_bodyHood;
    RenderSource *m_bodyTrunk;
    RenderSource *m_wheelSources[4];
    RenderSource *m_doorSources[4];
    std::vector<RenderParticleSource *> m_exhaustSources;
    std::vector<RenderParticleSource *> m_tireSmokeSources;

    TransformLinkRigidBody *m_linkBody;
    TransformLinkRigidBody *m_linkHood;
    TransformLinkRigidBody *m_linkTrunk;
    std::vector<TransformLinkWheel *> m_linkWheels;
    std::vector<TransformLinkDoor *> m_linkDoors;
    std::vector<TransformLinkRigidBody *> m_linkExhausts;
    std::vector<TransformLinkRigidBody *> m_linkTireSmokes;

    void update(float deltaTime);
    void updateExhaust(int index, float deltaTime);
    void updateTireSmoke(int index, float deltaTime);
    glm::mat4 translateOffset(glm::vec3 offset);
    void followCar(float deltaTime);
    void keyListener(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    int m_keyForward = GLFW_KEY_W;
    int m_keyBack = GLFW_KEY_S;
    int m_keyRight = GLFW_KEY_D;
    int m_keyLeft = GLFW_KEY_A;
    int m_keySpace = GLFW_KEY_SPACE;
};

#endif /* car_controller_hpp */
