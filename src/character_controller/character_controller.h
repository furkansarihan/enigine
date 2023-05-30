#ifndef character_controller_hpp
#define character_controller_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "../camera/camera.h"
#include "../ragdoll/ragdoll.h"
#include "../utils/common.h"
#include "../utils/bullet_glm.h"
#include "speed_limiter.h"

#include "btBulletDynamicsCommon.h"

struct ActionState
{
    bool left, right, forward, backward, run, jump;
};

class CharacterController
{
public:
    btDiscreteDynamicsWorld *m_dynamicsWorld;
    btRigidBody *m_rigidBody;
    Camera *m_followCamera;

    SpeedLimiter m_walkSpeed;
    SpeedLimiter m_runSpeed;
    float m_maxWalkRelative;
    float m_maxRunRelative;

    float m_moveForce = 4500.0f;
    float m_jumpForce = 300.0f;
    float m_speedAtJumpStart = 0.0f;
    float m_walkToRunAnimTreshold = 0.8f;
    float m_toIdleForce = 2250.0f;
    float m_toIdleForceHoriz = 900.0f;
    float m_elevationDistance = 0.0f;
    float m_worldElevation = 0.0f;
    float m_groundTreshold = 0.2f;
    float m_floatElevation = 0.05f;
    // turn
    float m_turnTreshold = 0.01f;
    float m_turnForce = 0.2f;
    float m_turnAnimForce = 0.1f;
    float m_turnFactor = 0.0f;
    float m_det = 0.0f;
    float m_turnTarget = 0.0f;
    float m_turnAnimMaxFactor = 0.4f;
    float m_turnAnimMult = 1.f;

    float m_halfHeight = 0.0f;

    float m_speed;
    btVector3 m_velocity;
    btVector3 m_verticalVelocity;
    float m_verticalSpeed;
    float m_horizontalSpeed;

    ActionState m_actionState;
    bool m_moving = false;
    bool m_onGround = false;
    bool m_falling = false;
    bool m_jumping = false;
    bool m_running = false;
    bool m_turning = false;
    bool m_turnLocked = false;

    glm::vec3 m_lookDir = glm::vec3(0.0f, 0.0f, 1.0f); // +Z
    float m_signedMoveAngle = M_PI; // +Z
    float m_signedMoveAngleTarget = M_PI;
    float m_moveAngleForce = 0.1f;
    float m_dotFront;

    CharacterController(btDiscreteDynamicsWorld *dynamicsWorld, btRigidBody *rigidBody, Camera *followCamera);
    ~CharacterController();
    void recieveInput(GLFWwindow *window, float deltaTime);
    void update(float deltaTime);
    void updateElevation();
    void updateVelocity();
};

#endif /* character_controller_hpp */
