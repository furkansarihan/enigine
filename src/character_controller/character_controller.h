#ifndef character_controller_hpp
#define character_controller_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "../camera/camera.h"

#include <GLFW/glfw3.h>
#include "btBulletDynamicsCommon.h"

class CharacterController
{
public:
    btDiscreteDynamicsWorld * m_dynamicsWorld;
    btRigidBody *m_rigidBody;
    Camera *m_followCamera;

    float m_moveForce = 30.0f;
    float m_jumpForce = 2.0f;
    float m_speedAtJumpStart = 0.0f;
    float m_maxWalkSpeed = 3.0f;
    float m_walkToRunAnimTreshold = 0.5f;
    float m_maxRunSpeed = 10.0f;
    float m_toIdleForce = 15.0f;
    float m_toIdleForceHoriz = 6.0f;
    float m_elevationDistance = 0.0f;
    float m_groundTreshold = 0.2f;
    // turn
    float m_turnTreshold = 0.01f;
    float m_turnForce = 0.1f;
    float m_turnAnimForce = 0.1f;
    float m_turnFactor = 0.0f;
    float m_turnTarget = 0.0f;
    float m_turnAnimMaxFactor = 0.4f;

    float m_halfHeight = 0.0f;

    float m_speed;
    btVector3 m_velocity;
    btVector3 m_verticalVelocity;
    float m_verticalSpeed;
    float m_horizontalSpeed;
    bool m_moving = false;
    bool m_onGround = false;
    bool m_falling = false;
    bool m_jumping = false;
    bool m_running = false;
    bool m_turning = false;

    glm::vec3 m_moveDir = glm::vec3(0.0f, 0.0f, 1.0f); // Z

    CharacterController(btDiscreteDynamicsWorld * dynamicsWorld, btRigidBody *rigidBody, Camera *followCamera);
    ~CharacterController();
    void update(GLFWwindow *window, float deltaTime);
    void updateElevation();
    void updateVelocity();
};

#endif /* character_controller_hpp */
