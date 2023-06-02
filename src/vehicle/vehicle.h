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

    PhysicsWorld *physicsWorld;
    btRigidBody *m_carChassis;
    btHinge2Constraint *wheels[4];
    btRigidBody *wheelBodies[4];

    float gEngineForce;
    float accelerationVelocity;
    float decreaseVelocity;
    float breakingVelocity;
    float maxEngineForce;
    float minEngineForce;
    float gVehicleSteering;
    float steeringIncrement;
    float steeringVelocity;
    float steeringClamp;
    float wheelRadius;
    float wheelWidth;

    float lowerLimit;
    float upperLimit;
    float damping;
    float friction;
    float stifness;
    float wheelDamping;
    float bounce;

    void update(GLFWwindow *window, float deltaTime);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

private:
    btVector3 position;
    void initDefaultValues();
    void initVehicle();
    void resetVehicle(btTransform tr);
    void updateSteering(GLFWwindow *window, float deltaTime);
    void updateAcceleration(GLFWwindow *window, float deltaTime);
};

#endif /* vehicle_hpp */
