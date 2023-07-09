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

struct RagdollSize
{
    float upperArmLength = 0.23f;
    float lowerArmLength = 0.25f;
    float shoulderOffsetVertical = 0.05f;
    float shoulderOffsetHorizontal = 0.22f;
    float upperLegLength = 0.38f;
    float lowerLegLength = 0.37f;
    float pelvisHeight = 0.23f;
    float spineHeight = 0.29f;
    float headHeight = 0.05f;

    bool operator==(const RagdollSize &other) const
    {
        return upperArmLength == other.upperArmLength &&
               lowerArmLength == other.lowerArmLength &&
               shoulderOffsetVertical == other.shoulderOffsetVertical &&
               shoulderOffsetHorizontal == other.shoulderOffsetHorizontal &&
               upperLegLength == other.upperLegLength &&
               lowerLegLength == other.lowerLegLength &&
               pelvisHeight == other.pelvisHeight &&
               spineHeight == other.spineHeight &&
               headHeight == other.headHeight;
    }
};

class Ragdoll
{

public:
    Ragdoll(PhysicsWorld *physicsWorld, Animation *animation, const btVector3 &positionOffset, btScalar scale);
    ~Ragdoll();

    PhysicsWorld *m_physicsWorld;
    Animation *m_animation;
    btScalar m_scale;
    btCapsuleShape *m_shapes[BODYPART_COUNT];
    btRigidBody *m_bodies[BODYPART_COUNT];
    btTypedConstraint *m_joints[JOINT_COUNT];

    JointTarget m_fetalTargets[JOINT_COUNT];
    RagdollStatus m_status;
    RagdollSize m_size;

    // debug
    btQuaternion orients[JOINT_COUNT];
    btQuaternion parentOrients[JOINT_COUNT];
    btQuaternion diffs[JOINT_COUNT];
    btQuaternion nodeOrients[JOINT_COUNT];
    btQuaternion boneOrients[JOINT_COUNT];
    btQuaternion skips[JOINT_COUNT];
    Bone *bones[JOINT_COUNT];
    btTransform boneTransforms[JOINT_COUNT];

    glm::vec3 m_modelOffset = glm::vec3(0.0f, -1.9f, 0);

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
    void syncNodeToAnimation(AssimpNodeData node, btQuaternion skipNodeOrientation, btQuaternion parentBoneOrientation, glm::vec3 &position, bool fromParent);

private:
    void updateStateChange();
};

#endif /* ragdoll_hpp */
