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

class CarController
{
public:
    CarController(PhysicsWorld *physicsWorld, ResourceManager *resourceManager, Camera *followCamera, glm::vec3 position);
    ~CarController();

    Vehicle *m_vehicle;
    Camera *m_followCamera;
    ParticleEngine *m_exhausParticle;

    float m_scale = 0.028f;
    float m_wheelScale = 0.028f;
    glm::vec3 m_bodyOffset = glm::vec3(0.f, 2.07f, -0.14f);
    glm::vec3 m_wheelOffset = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 m_rotation = glm::vec3(0.f, -M_PI_2, 0.f);
    glm::vec3 m_wheelRotation = glm::vec3(0.f, -M_PI_2, 0.f);
    glm::vec3 m_bodyRotation = glm::vec3(0.f, 0.f, -0.020f);

    glm::vec3 m_hoodOffset = glm::vec3(0.f, 1.87f, 3.12f);
    glm::vec3 m_trunkOffset = glm::vec3(0.f, 2.15f, -3.89f);
    glm::vec3 m_doorOffsets[4];

    glm::vec3 m_exhaustOffset = glm::vec3(0.98f, 0.93f, -4.34f);
    glm::vec3 m_exhaustRotation = glm::vec3(-0.9f, -2.25f, 0.f);

    // entering to car
    glm::vec2 m_safeSize = glm::vec2(1.f, 3.f);
    glm::vec2 m_doorOffset = glm::vec2(3.f, 0.f);

    bool m_controlVehicle = false;
    bool m_followVehicle = false;

    glm::vec3 m_pos = glm::vec3(0.0f, 2.5f, 0.0f);
    Follow m_follow;

    glm::mat4 m_carModel;

    void update(GLFWwindow *window, float deltaTime);
    void updateExhaust(GLFWwindow *window, float deltaTime);
    glm::mat4 translateOffset(glm::vec3 offset);
    void updateModels();
    void followCar();
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};

#endif /* car_controller_hpp */
