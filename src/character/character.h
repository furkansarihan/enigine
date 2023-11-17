#ifndef character_hpp
#define character_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <stack>
#include <future>
#include <atomic>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "btBulletDynamicsCommon.h"

#include "../external/jps3d/include/jps_planner/jps_planner/jps_planner.h"
#include "../external/jps3d/include/jps_planner/distance_map_planner/distance_map_planner.h"

#include "../camera/camera.h"
#include "../model/model.h"
#include "../ragdoll/ragdoll.h"
#include "../character_controller/character_controller.h"
#include "../physics_world/physics_world.h"
#include "../utils/common.h"
#include "../utils/bullet_glm.h"
#include "../shader_manager/shader_manager.h"
#include "../resource_manager/resource_manager.h"
#include "../car_controller/car_controller.h"
class TaskManager;
#include "../task_manager/task_manager.h"
#include "../character_task/follow_path.h"
#include "../character_task/enter_car.h"
#include "../character_task/exit_car.h"
#include "../render_manager/render_manager.h"
#include "../update_manager/update_manager.h"

struct PathResult
{
    bool empty = true;
    Vec2i dim;
    JPS::Tmap data;
    Vec2f start;
    Vec2f startWorld;
    Vec2f goal;
    vec_Vec2f path;
};

enum PassengerState
{
    outside,
    entering,
    inside,
    exiting,
    exitInterrupt
};

struct PassengerInfo
{
    PassengerState state = PassengerState::outside;
    bool exitRequested = false;
    CarController *car = nullptr;
};

class Character : public Updatable
{
public:
    RenderManager *m_renderManager;
    RenderSource *m_renderSource;
    TaskManager *m_taskManager;
    ResourceManager *m_resourceManager;
    ShaderManager *m_shaderManager;
    PhysicsWorld *m_physicsWorld;
    Camera *m_followCamera;
    CharacterController *m_controller;
    Animator *m_animator;
    Ragdoll *m_ragdoll;
    btRigidBody *m_rigidbody;
    Model *m_model;
    PathResult m_lastCarEnterPath;
    PassengerInfo m_passengerInfo;
    bool m_syncPositionFromPhysics = true;

    RenderSource *m_transform;
    glm::mat4 m_modelMatrix;
    glm::vec3 m_position = glm::vec3(200.f, 5.f, 250.f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_scale = 1.0;

    // animation
    float m_blendTargets[13];
    float m_blendTargetsPose[2];
    float m_blendSpeed = 0.2f;

    float m_runFactor = 0.f;
    float m_idleFactor = 0.f;
    float m_prevRunFactor = 0.f;
    float m_prevIdleFactor = 0.f;

    float m_walkStepFreq = 1.f;
    float m_runStepFreq = 1.f;

    float m_stopBlendSpeed = 0.022f;

    float m_walkPerc = 0.f;
    float m_runPerc = 0.f;

    float m_walkAnimSpeed = 0.f;
    float m_runAnimSpeed = 0.f;

    float m_leftBlendEdge = 1.45f;
    float m_rightBlendEdge = 1.2f;
    float m_leftForward = 0.35f;
    float m_rightForward = 0.05f;

    float m_aimBlend = 0.0f;
    float m_aimStateChangeSpeed = 3.f;

    float m_firingBlend = 0.0f;
    float m_firingStateChangeSpeed = 3.f;

    std::atomic_bool m_firing;

    // ragdoll
    bool m_ragdollActive = false;
    float m_impulseStrength = 600.f;
    float m_ragdolActivateThreshold = 3000.f;
    float m_ragdolActivateFactor = 0.1f;
    float m_stateChangeSpeed = 10.f;

    glm::quat m_neckRot;
    glm::quat m_headRot;
    glm::quat m_clampedHeadRot;
    glm::quat m_headRotOffset;

    bool m_controlCharacter;
    bool m_followCharacter;
    bool m_headFollow;
    bool m_lastHeadFollow;

    Character(RenderManager *renderManager, TaskManager *taskManager, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~Character();
    void init();
    void update(float deltaTime);
    void updateHeadFollow(float deltaTime);
    bool updateHeadFollowRotation(bool firstUpdate);
    glm::quat getNeckRotation();
    void updateModelMatrix();
    void interpolateBlendTargets();
    void syncFootstepFrequency();
    void setWalkPlaybackSpeed(float animSpeed);
    void setRunPlaybackSpeed(float animSpeed);
    void setWalkTimer(float time);
    void setRunTimer(float time);
    void activateRagdoll();
    void applyImpulseFullRagdoll(glm::vec3 impulse);
    void applyImpulseChest(glm::vec3 impulse);
    void resetRagdoll();
    void activateCollider();
    void inactivateCollider();
    void checkPhysicsStateChange();
    AnimPose &getRagdolPose();
    AnimPose &getAimPose();
    AnimPose &getFiringPose();
    AnimPose &getEnterCarAnim();
    AnimPose &getExitCarAnim();
    void updateAimPoseBlendMask(float blendFactor);
    void enterNearestCar();
    void cancelEnterCar();
    void interruptExitCar();
    void exitFromCar();
};

#endif /* character_hpp */
