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
    glm::vec3 offset = glm::vec3(0.0f, 2.5f, 0.0f);
    float distance = 15.0f;
    float gap = 0.0f;
    float gapTarget = 0.0f;
    float gapFactor = 0.1f;
    float gapSpeed = 0.002f;
    float steeringFactor = 30.0f;
    float angleFactor = 0.16f;
    float angleSpeed = 0.011f;
    float angleVelocity = 0.f;
    float angleVelocityTarget = 0.f;
    float angleVelocitySpeed = 0.02f;
    float move = 0.f;
    float moveTarget = 0.f;
    float moveSpeed = 0.005f;
    float moveSpeedRange = 20.f;
    float angularSpeedRange = 50.f;
};

struct Models
{
    Model *carBody;
    Model *carHood;
    Model *carTrunk;
    Model *carWheelFL;
    Model *carWheelFR;
    Model *carWheelRL;
    Model *carWheelRR;
    Model *wheelModels[4];
    Model *carDoorFL;
    Model *carDoorFR;
    Model *carDoorRL;
    Model *carDoorRR;
    Model *doorModels[4];
};

class CarController : public Updatable
{
public:
    CarController(GLFWwindow *window, ShaderManager *shaderManager, RenderManager *renderManager, PhysicsWorld *physicsWorld, ResourceManager *resourceManager, Camera *followCamera, glm::vec3 position);
    ~CarController();

    GLFWwindow *m_window;
    Vehicle *m_vehicle;
    Camera *m_followCamera;
    ParticleEngine *m_exhausParticle;
    Models m_models;
    Shader m_exhaustShader;

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
    RenderParticleSource *m_exhaustSource;

    void update(float deltaTime);
    void updateExhaust(float deltaTime);
    glm::mat4 translateOffset(glm::vec3 offset);
    void followCar();
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    int m_keyForward = GLFW_KEY_W;
    int m_keyBack = GLFW_KEY_S;
    int m_keyRight = GLFW_KEY_D;
    int m_keyLeft = GLFW_KEY_A;
};

#endif /* car_controller_hpp */
