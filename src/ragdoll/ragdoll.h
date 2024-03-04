#ifndef ragdoll_hpp
#define ragdoll_hpp

#include "btBulletDynamicsCommon.h"

#include "../physics_world/physics_world.h"
#include "../animation/animation.h"
#include "../animation/animator.h"
#include "../animation/bone.h"
#include "../utils/bullet_glm.h"

enum
{
    BODYPART_PELVIS = 0,
    BODYPART_SPINE,
    BODYPART_HEAD,
    BODYPART_LEFT_UPPER_LEG,
    BODYPART_LEFT_LOWER_LEG,
    BODYPART_RIGHT_UPPER_LEG,
    BODYPART_RIGHT_LOWER_LEG,
    BODYPART_LEFT_UPPER_ARM,
    BODYPART_LEFT_LOWER_ARM,
    BODYPART_RIGHT_UPPER_ARM,
    BODYPART_RIGHT_LOWER_ARM,
    BODYPART_COUNT
};

enum
{
    JOINT_PELVIS_SPINE = 0,
    JOINT_SPINE_HEAD,
    JOINT_LEFT_HIP,
    JOINT_LEFT_KNEE,
    JOINT_RIGHT_HIP,
    JOINT_RIGHT_KNEE,
    JOINT_LEFT_SHOULDER,
    JOINT_LEFT_ELBOW,
    JOINT_RIGHT_SHOULDER,
    JOINT_RIGHT_ELBOW,
    JOINT_COUNT
};

struct JointTarget
{
    bool active = false;
    glm::vec4 angle = glm::vec4(0.f); // quat or float as .x
    float force = 0.f;
};

enum RagdollState
{
    loose,
    fetal
};

struct RagdollStatus
{
    RagdollState state = RagdollState::loose;
    RagdollState prevState = RagdollState::loose;
};

struct RagdollNodeData
{
    AssimpNodeData *animNode;
    glm::mat4 transform;

    int jointIndex = 0;
    int boneIndex = 0;
    btRigidBody *rigidBody = nullptr;
    std::vector<RagdollNodeData *> childNodes;
    RagdollNodeData *parentNode = nullptr;

    //
    btQuaternion boneRot = btQuaternion::getIdentity();
    btQuaternion nodeRot = btQuaternion::getIdentity();
    btQuaternion relativeNodeRot = btQuaternion::getIdentity();
};

struct AnimationNode
{
    AssimpNodeData *animNode = nullptr;
    glm::mat4 transform;
};

struct RagdollSize
{
    float upperArmLength = 0.29f;
    float lowerArmLength = 0.25f;
    glm::vec3 shoulderOffset = glm::vec3(0.193f, 0.035f, -0.030f);
    float upperLegLength = 0.38f;
    float lowerLegLength = 0.37f;
    float pelvisHeight = 0.23f;
    float spineHeight = 0.28f;
    float headHeight = 0.05f;

    bool operator==(const RagdollSize &other) const
    {
        return upperArmLength == other.upperArmLength &&
               lowerArmLength == other.lowerArmLength &&
               shoulderOffset == other.shoulderOffset &&
               upperLegLength == other.upperLegLength &&
               lowerLegLength == other.lowerLegLength &&
               pelvisHeight == other.pelvisHeight &&
               spineHeight == other.spineHeight &&
               headHeight == other.headHeight;
    }

    bool operator!=(const RagdollSize &other) const
    {
        return upperArmLength != other.upperArmLength ||
               lowerArmLength != other.lowerArmLength ||
               shoulderOffset != other.shoulderOffset ||
               upperLegLength != other.upperLegLength ||
               lowerLegLength != other.lowerLegLength ||
               pelvisHeight != other.pelvisHeight ||
               spineHeight != other.spineHeight ||
               headHeight != other.headHeight;
    }
};

class Ragdoll
{

public:
    Ragdoll(PhysicsWorld *physicsWorld, Animator *animator, Animation *animation, const btVector3 &positionOffset, btScalar scale);
    ~Ragdoll();

    PhysicsWorld *m_physicsWorld;
    Animator *m_animator;
    Animation *m_animation;
    btScalar m_scale;
    btCapsuleShape *m_shapes[BODYPART_COUNT];
    btRigidBody *m_bodies[BODYPART_COUNT];
    btTypedConstraint *m_joints[JOINT_COUNT];

    JointTarget m_fetalTargets[JOINT_COUNT];
    RagdollStatus m_status;
    RagdollSize m_size;

    RagdollNodeData nodeRoot;
    RagdollNodeData nodePelvis;
    RagdollNodeData nodeSpine;
    RagdollNodeData nodeHead;
    RagdollNodeData nodeLeftUpLeg;
    RagdollNodeData nodeLeftLeg;
    RagdollNodeData nodeRightUpLeg;
    RagdollNodeData nodeRightLeg;
    RagdollNodeData nodeLeftArm;
    RagdollNodeData nodeLeftForeArm;
    RagdollNodeData nodeRightArm;
    RagdollNodeData nodeRightForeArm;

    // to-animation offsets
    glm::vec3 m_modelOffset;
    btQuaternion m_pelvisOffset;
    btQuaternion m_spineOffset;
    btQuaternion m_headOffset;
    btQuaternion m_rightArmOffset;
    btQuaternion m_leftArmOffset;
    btQuaternion m_rightForeArmOffset;
    btQuaternion m_leftForeArmOffset;
    btQuaternion m_leftLegOffset;
    btQuaternion m_rightLegOffset;

    // from-animation offsets
    glm::quat m_legOffset;
    glm::vec3 m_armatureScale;

    void resetTransforms(const btVector3 &offsetPosition, btQuaternion offsetRotation);
    void resetTransforms(const btVector3 &offsetPosition, float angleY);
    void freezeBodies();
    void unFreezeBodies();
    void update(float deltaTime);
    void changeState(RagdollState newState);
    void updateJointFrames();
    void updateJointSizes();
    void updateJointSize(btCapsuleShape *shape, btRigidBody *body, float size);

    void syncToAnimation(glm::vec3 &position);
    void syncNodeToAnimation(RagdollNodeData *node, btQuaternion offset, bool flipVertical = false);
    void syncFromAnimation(glm::mat4 characterModel);
    void syncNodeFromAnimation(const RagdollNodeData &node, glm::mat4 characterModel, glm::quat offset, bool offsetSize = true);

private:
    void updateStateChange();

    AnimationNode getNode(AssimpNodeData *node, std::string name, glm::mat4 parentTransform);
    void setupNode(RagdollNodeData &node, RagdollNodeData *parentNode, const std::string &name, int bodyPart, int joint);
};

#endif /* ragdoll_hpp */
