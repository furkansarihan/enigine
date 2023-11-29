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

struct MoveCircle
{
    Anim *m_forward;
    Anim *m_back;
    Anim *m_left;
    Anim *m_right;
    Anim *m_backLeft;
    Anim *m_backRight;
};

struct MoveOrient
{
    float forward;
    float back;
    float left;
    float right;
    float backLeft;
    float backRight;
};

struct MoveStage
{
    float idle;
    float walk;
    float run;

    float idleEndGap = 0.1f;
    float walkStartGap = 0.1;
    float walkEndGap = 0.1f;
    float runStartGap = 0.1f;
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
    glm::vec3 m_position = glm::vec3(125.f, 5.f, 250.f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_scale = 1.0;

    // animation states
    Anim *m_idle;
    MoveCircle m_walkCircle;
    MoveCircle m_runCircle;
    MoveOrient m_moveOrient;
    MoveStage m_moveStage;
    MoveStage m_prevMoveStage;

    // animation poses
    Anim *m_animPoseLeanLeft;
    Anim *m_animPoseLeanRight;
    Anim *m_animPoseRagdoll;
    Anim *m_animPosePistolAim;
    Anim *m_animPoseFiring;
    Anim *m_animPoseEnterCar;
    Anim *m_animPoseExitCar;
    Anim *m_animPoseJumpCar;
    Anim *m_animPoseHeadFollow;
    Anim *m_animTurn180;

    float m_blendSpeed = 0.2f;

    float m_walkStepFreq = 1.f;
    float m_runStepFreq = 1.f;

    float m_stopBlendSpeed = 0.022f;

    float m_walkPerc = 0.f;
    float m_runPerc = 0.f;

    float m_walkAnimSpeed = 0.f;
    float m_runAnimSpeed = 0.f;

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
    void updateMoveOrient();
    void updateMoveStage();
    void updateMoveCircleBlend(MoveCircle &circle, float value);
    void updateMoveCircleTimer(MoveCircle &circle, float value);
    void updateMoveCirclePlaybackSpeed(MoveCircle &circle, float value);
    void assignBlend(float &orient, float dot);
    void interpolateValue(float &value, float newValue);
    void updateHeadFollow(float deltaTime);
    bool updateHeadFollowRotation(bool firstUpdate);
    glm::quat getNeckRotation();
    void updateModelMatrix();
    void interpolateBlendTargets();
    void syncFootstepFrequency();
    void syncFootstepFrequency2();
    void activateRagdoll();
    void applyImpulseFullRagdoll(glm::vec3 impulse);
    void applyImpulseChest(glm::vec3 impulse);
    void resetRagdoll();
    void activateCollider();
    void inactivateCollider();
    void checkPhysicsStateChange();
    void updateAimPoseBlendMask(float blendFactor);
    void enterNearestCar();
    void cancelEnterCar();
    void interruptExitCar();
    void exitFromCar();
};

#endif /* character_hpp */
