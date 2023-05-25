#include "character.h"

Character::Character(ShaderManager *shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : m_shaderManager(shaderManager),
      m_physicsWorld(physicsWorld),
      m_followCamera(followCamera)
{
    init();
}

void Character::init()
{
    // Animation
    // TODO: single aiScene read
    m_model = new Model("../src/assets/gltf/char2.glb");
    Animation *animation0 = new Animation("../src/assets/gltf/char2.glb", "idle", m_model);
    Animation *animation1 = new Animation("../src/assets/gltf/char2.glb", "walking", m_model);
    Animation *animation2 = new Animation("../src/assets/gltf/char2.glb", "left", m_model);
    Animation *animation3 = new Animation("../src/assets/gltf/char2.glb", "right", m_model);
    Animation *animation4 = new Animation("../src/assets/gltf/char2.glb", "running", m_model);
    // TODO: create empty at runtime?
    Animation *animationRagdoll = new Animation("../src/assets/gltf/char2.glb", "pose", m_model);

    // TODO: inside Model
    std::vector<Animation *> animations;
    animations.push_back(animation0);
    animations.push_back(animation1);
    animations.push_back(animation2);
    animations.push_back(animation3);
    animations.push_back(animation4);
    animations.push_back(animationRagdoll);

    // TODO: setup multiple animators from same Model
    m_animator = new Animator(animations);

    // idle - walk
    // TODO: increase accessibility - name or pointer to Animation
    m_animator->m_state.fromIndex = 0;
    m_animator->m_state.toIndex = 1;
    m_animator->m_state.blendFactor = 0.0f;

    // set blend mask for turn-right and turn-left
    std::unordered_map<std::string, float> blendMask;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;

    animation2->setBlendMask(blendMask);
    animation3->setBlendMask(blendMask);
    // TODO: ragdoll mask for ragdoll hands - default position?

    // turn-left pose
    AnimPose animPose;
    animPose.index = 2;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // turn-right pose
    animPose.index = 3;
    m_animator->m_state.poses.push_back(animPose);
    // ragdoll
    animPose.index = 5;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);

    // Character
    m_rigidbody = m_physicsWorld->createCapsule(10.0f, 1.0f, 0.5f, 2.0f, BulletGLM::getBulletVec3(m_position));
    m_rigidbody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setDamping(0.9f, 0.9f);
    m_rigidbody->setFriction(0.0f);
    m_rigidbody->setGravity(btVector3(0, -20.0f, 0));

    m_controller = new CharacterController(m_physicsWorld->dynamicsWorld, m_rigidbody, m_followCamera);

    // TODO: make independent or manageable - fix
    // init shaders after model read && before ragdoll ???? :D
    m_shaderManager->initShaders();

    m_ragdoll = new Ragdoll(m_physicsWorld, animationRagdoll, BulletGLM::getBulletVec3(m_position), 2.0f);
}

Character::~Character()
{
    delete m_controller;
    for (int i = 0; i < m_animator->m_animations.size(); i++)
        delete m_animator->m_animations[i];
    m_animator->m_animations.clear();
    delete m_animator;
    delete m_ragdoll;
    delete m_model;
}

void Character::update(float deltaTime)
{
    // update character
    m_controller->update(deltaTime);

    // update ragdoll
    if (m_controller->m_ragdollActive)
        m_ragdoll->syncToAnimation(m_position);

    // update animation
    m_animator->update(deltaTime);

    // update animator blend
    m_animator->m_state.poses[2].blendFactor += deltaTime * m_controller->m_stateChangeSpeed * (m_controller->m_ragdollActive ? 1.f : -1.f);
    float clamped = std::max(0.0f, std::min(m_animator->m_state.poses[2].blendFactor, 1.0f));
    m_animator->m_state.poses[2].blendFactor = clamped;

    // sync animator with controller
    if (m_rigidbody && m_rigidbody->getMotionState() && !m_controller->m_ragdollActive)
    {
        btTransform trans;
        m_rigidbody->getMotionState()->getWorldTransform(trans);
        m_position = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
        m_position -= glm::vec3(0, m_controller->m_halfHeight, 0);
        m_rotation.y = glm::atan(m_controller->m_moveDir.x, m_controller->m_moveDir.z);

        float maxWalkSpeed = m_controller->m_maxWalkSpeed + m_controller->m_walkToRunAnimTreshold;
        if (m_controller->m_verticalSpeed > maxWalkSpeed)
        {
            float runnningGap = m_controller->m_maxRunSpeed - maxWalkSpeed;
            float runningLevel = m_controller->m_verticalSpeed - maxWalkSpeed;
            float clamped = CommonUtil::snappedClamp(runningLevel / runnningGap, 0.0f, 1.0f, 0.08f);
            m_animator->m_state.blendFactor = clamped;
            m_animator->m_state.fromIndex = 1;
            m_animator->m_state.toIndex = 4;
        }
        else
        {
            float clamped = CommonUtil::snappedClamp(m_controller->m_verticalSpeed / m_controller->m_maxWalkSpeed, 0.0f, 1.0f, 0.08f);
            m_animator->m_state.blendFactor = clamped;
            m_animator->m_state.fromIndex = 0;
            m_animator->m_state.toIndex = 1;
        }

        AnimPose *animL = &m_animator->m_state.poses[0];
        AnimPose *animR = &m_animator->m_state.poses[1];
        animL->blendFactor = std::max(0.0f, std::min(-m_controller->m_turnFactor, 1.0f));
        animR->blendFactor = std::max(0.0f, std::min(m_controller->m_turnFactor, 1.0f));
    }
}
