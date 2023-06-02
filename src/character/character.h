#ifndef character_hpp
#define character_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/gtx/rotate_vector.hpp>
#include "btBulletDynamicsCommon.h"

#include "../camera/camera.h"
#include "../model/model.h"
#include "../ragdoll/ragdoll.h"
#include "../character_controller/character_controller.h"
#include "../physics_world/physics_world.h"
#include "../utils/common.h"
#include "../utils/bullet_glm.h"
#include "../shader_manager/shader_manager.h"

class Character
{
public:
    ShaderManager *m_shaderManager;
    PhysicsWorld *m_physicsWorld;
    Camera *m_followCamera;
    CharacterController *m_controller;
    Animator *m_animator;
    Ragdoll *m_ragdoll;
    btRigidBody *m_rigidbody;
    Model *m_model;

    // TODO: transformation struct
    glm::vec3 m_position = glm::vec3(200.0f, 10.5f, 250.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_scale = 2.0;

    // animation
    float m_blendTargets[13];
    float m_blendSpeed = 0.2f;

    float m_leftBlendEdge = 1.45f;
    float m_rightBlendEdge = 1.2f;
    float m_leftForward = 0.35f;
    float m_rightForward = 0.05f;

    float m_aimBlend = 0.0f;

    // ragdoll
    bool m_ragdollActive = false;
    float m_impulseStrength = 600.f;
    float m_stateChangeSpeed = 10.f;

    Character(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~Character();
    void init();
    void update(float deltaTime);
    void interpolateBlendTargets();
    void activateRagdoll(glm::vec3 impulseDirection, float impulseStrength);
    void resetRagdoll();
    AnimPose &getRagdolPose();
    AnimPose &getAimPose();
    void updateAimPoseBlendMask(float blendFactor);
};

#endif /* character_hpp */
