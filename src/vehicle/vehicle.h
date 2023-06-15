#ifndef vehicle_hpp
#define vehicle_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "../physics_world/physics_world.h"
#include "../utils/common.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"

class Vehicle
{
public:
    Vehicle(PhysicsWorld *physicsWorld, btVector3 position);
    ~Vehicle();

    PhysicsWorld *m_physicsWorld;
    btRigidBody *m_carChassis;

    btDefaultVehicleRaycaster *m_vehicleRayCaster;
    btRaycastVehicle *m_vehicle;
    btRaycastVehicle::btVehicleTuning m_tuning;

    float m_wheelFriction;
    float m_suspensionStiffness;
    float m_suspensionDamping;
    float m_suspensionCompression;
    float m_rollInfluence;
    float m_suspensionRestLength;

    float m_speed = 0.f;
    bool m_inAction = false;
    float gEngineForce;
    float accelerationVelocity;
    float decreaseVelocity;
    float breakingVelocity;
    float maxEngineForce;
    float minEngineForce;
    float gVehicleSteering;
    float steeringIncrement;
    float steeringSpeed;
    float steeringClamp;
    float wheelRadius;

    void update(GLFWwindow *window, float deltaTime);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    btVector3 m_position;
    int m_keyForward = GLFW_KEY_W;
    int m_keyBack = GLFW_KEY_S;
    int m_keyRight = GLFW_KEY_D;
    int m_keyLeft = GLFW_KEY_A;

    void initDefaultValues();
    void initVehicle();
    void resetVehicle(btTransform tr);
    void updateSteering(GLFWwindow *window, float deltaTime);
    void updateAcceleration(GLFWwindow *window, float deltaTime);
};

#endif /* vehicle_hpp */
