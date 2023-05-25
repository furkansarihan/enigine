#ifndef character_hpp
#define character_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "../camera/camera.h"
#include "../model/model.h"
#include "../ragdoll/ragdoll.h"
#include "../character_controller/character_controller.h"
#include "../physics_world/physics_world.h"
#include "../utils/common.h"
#include "../utils/bullet_glm.h"
#include "../shader_manager/shader_manager.h"

#include <GLFW/glfw3.h>
#include "btBulletDynamicsCommon.h"

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
    glm::vec3 m_position = glm::vec3(200.0f, 10.5f, 200.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_scale = 2.0;

    Character(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~Character();
    void init();
    void update(float deltaTime);
};

#endif /* character_hpp */
