#include "ragdoll.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../utils/common.h"

Ragdoll::Ragdoll(PhysicsWorld *physicsWorld, Animator *animator, Animation *animation, const btVector3 &positionOffset, btScalar scale)
    : m_physicsWorld(physicsWorld),
      m_animator(animator),
      m_animation(animation),
      m_scale(scale),
      m_modelOffset(glm::vec3(0.0f)),
      m_pelvisOffset(btQuaternion(0.f, 0.f, 0.f, 1.f)),
      m_spineOffset(btQuaternion(0.f, 0.f, 0.f, 1.f)),
      m_headOffset(btQuaternion(0.f, 0.f, 0.f, 1.f)),
      m_rightArmOffset(btQuaternion(0.440f, -0.565f, 0.447f, -0.536f)),
      m_leftArmOffset(btQuaternion(0.382f, 0.625f, -0.488f, -0.474f)),
      m_rightForeArmOffset(btQuaternion(0.f, 0.f, 1.f, 0.f)),
      m_leftForeArmOffset(btQuaternion(0.f, 0.f, 1.f, 0.f)),
      m_leftLegOffset(btQuaternion(0.f, 0.f, 0.f, 1.f)),
      m_rightLegOffset(btQuaternion(0.f, 0.f, 0.f, 1.f)),
      m_legOffset(glm::quat(0.f, 0.f, 0.f, 1.f)),
      m_armatureScale(glm::vec3(100.f))
{
    m_shapes[BODYPART_PELVIS] = new btCapsuleShape(btScalar(0.15) * m_scale, btScalar(m_size.pelvisHeight) * m_scale);
    m_shapes[BODYPART_SPINE] = new btCapsuleShape(btScalar(0.15) * m_scale, btScalar(m_size.spineHeight) * m_scale);
    m_shapes[BODYPART_HEAD] = new btCapsuleShape(btScalar(0.10) * m_scale, btScalar(m_size.headHeight) * m_scale);
    m_shapes[BODYPART_LEFT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07) * m_scale, btScalar(m_size.upperLegLength) * m_scale);
    m_shapes[BODYPART_LEFT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(m_size.lowerLegLength) * m_scale);
    m_shapes[BODYPART_RIGHT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07) * m_scale, btScalar(m_size.upperLegLength) * m_scale);
    m_shapes[BODYPART_RIGHT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(m_size.lowerLegLength) * m_scale);
    m_shapes[BODYPART_LEFT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(m_size.upperArmLength) * m_scale);
    m_shapes[BODYPART_LEFT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04) * m_scale, btScalar(m_size.lowerArmLength) * m_scale);
    m_shapes[BODYPART_RIGHT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(m_size.upperArmLength) * m_scale);
    m_shapes[BODYPART_RIGHT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04) * m_scale, btScalar(m_size.lowerArmLength) * m_scale);

    btScalar mass(80.f);
    btTransform transform;
    transform.setIdentity();

    m_bodies[BODYPART_PELVIS] = physicsWorld->createRigidBody(m_shapes[BODYPART_PELVIS], mass * 0.20f, transform);
    m_bodies[BODYPART_SPINE] = physicsWorld->createRigidBody(m_shapes[BODYPART_SPINE], mass * 0.32f, transform);
    m_bodies[BODYPART_HEAD] = physicsWorld->createRigidBody(m_shapes[BODYPART_HEAD], mass * 0.08f, transform);
    m_bodies[BODYPART_LEFT_UPPER_LEG] = physicsWorld->createRigidBody(m_shapes[BODYPART_LEFT_UPPER_LEG], mass * 0.08f, transform);
    m_bodies[BODYPART_LEFT_LOWER_LEG] = physicsWorld->createRigidBody(m_shapes[BODYPART_LEFT_LOWER_LEG], mass * 0.07f, transform);
    m_bodies[BODYPART_RIGHT_UPPER_LEG] = physicsWorld->createRigidBody(m_shapes[BODYPART_RIGHT_UPPER_LEG], mass * 0.08f, transform);
    m_bodies[BODYPART_RIGHT_LOWER_LEG] = physicsWorld->createRigidBody(m_shapes[BODYPART_RIGHT_LOWER_LEG], mass * 0.07f, transform);
    m_bodies[BODYPART_LEFT_UPPER_ARM] = physicsWorld->createRigidBody(m_shapes[BODYPART_LEFT_UPPER_ARM], mass * 0.03f, transform);
    m_bodies[BODYPART_LEFT_LOWER_ARM] = physicsWorld->createRigidBody(m_shapes[BODYPART_LEFT_LOWER_ARM], mass * 0.02f, transform);
    m_bodies[BODYPART_RIGHT_UPPER_ARM] = physicsWorld->createRigidBody(m_shapes[BODYPART_RIGHT_UPPER_ARM], mass * 0.03f, transform);
    m_bodies[BODYPART_RIGHT_LOWER_ARM] = physicsWorld->createRigidBody(m_shapes[BODYPART_RIGHT_LOWER_ARM], mass * 0.02f, transform);

    // TODO: run syncFromAnimation for initial position
    resetTransforms(positionOffset, 0.f);
    freezeBodies();

    for (int i = 0; i < BODYPART_COUNT; ++i)
    {
        m_bodies[i]->setGravity(btVector3(0, -20.0f, 0));
        m_bodies[i]->setFriction(btScalar(0.8));
        m_bodies[i]->setDamping(btScalar(0.05), btScalar(0.85));
        m_bodies[i]->setDeactivationTime(btScalar(0.8));
        m_bodies[i]->setSleepingThresholds(btScalar(1.6), btScalar(2.5));
    }

    // setup animation nodes
    setupNode(nodePelvis, &nodeRoot, "mixamorig:Hips", BODYPART_PELVIS, -1);
    setupNode(nodeSpine, &nodePelvis, "mixamorig:Spine1", BODYPART_SPINE, JOINT_PELVIS_SPINE);
    setupNode(nodeHead, &nodeSpine, "mixamorig:Head", BODYPART_HEAD, JOINT_SPINE_HEAD);

    setupNode(nodeLeftUpLeg, &nodePelvis, "mixamorig:LeftUpLeg", BODYPART_LEFT_UPPER_LEG, JOINT_LEFT_HIP);
    setupNode(nodeLeftLeg, &nodeLeftUpLeg, "mixamorig:LeftLeg", BODYPART_LEFT_LOWER_LEG, JOINT_LEFT_KNEE);
    setupNode(nodeRightUpLeg, &nodePelvis, "mixamorig:RightUpLeg", BODYPART_RIGHT_UPPER_LEG, JOINT_RIGHT_HIP);
    setupNode(nodeRightLeg, &nodeRightUpLeg, "mixamorig:RightLeg", BODYPART_RIGHT_LOWER_LEG, JOINT_RIGHT_KNEE);

    setupNode(nodeLeftArm, &nodeSpine, "mixamorig:LeftArm", BODYPART_LEFT_UPPER_ARM, JOINT_LEFT_SHOULDER);
    setupNode(nodeLeftForeArm, &nodeLeftArm, "mixamorig:LeftForeArm", BODYPART_LEFT_LOWER_ARM, JOINT_LEFT_ELBOW);
    setupNode(nodeRightArm, &nodeSpine, "mixamorig:RightArm", BODYPART_RIGHT_UPPER_ARM, JOINT_RIGHT_SHOULDER);
    setupNode(nodeRightForeArm, &nodeRightArm, "mixamorig:RightForeArm", BODYPART_RIGHT_LOWER_ARM, JOINT_RIGHT_ELBOW);

    // tree setup
    nodePelvis.childNodes.push_back(&nodeSpine);
    nodePelvis.childNodes.push_back(&nodeLeftUpLeg);
    nodePelvis.childNodes.push_back(&nodeRightUpLeg);

    nodeSpine.childNodes.push_back(&nodeHead);
    nodeSpine.childNodes.push_back(&nodeLeftArm);
    nodeSpine.childNodes.push_back(&nodeRightArm);

    nodeLeftUpLeg.childNodes.push_back(&nodeLeftLeg);
    nodeRightUpLeg.childNodes.push_back(&nodeRightLeg);

    nodeLeftArm.childNodes.push_back(&nodeLeftForeArm);
    nodeRightArm.childNodes.push_back(&nodeRightForeArm);

    // constraints
    btHingeConstraint *hingeC;
    btConeTwistConstraint *coneC;

    btTransform localA, localB;
    localA.setIdentity();
    localB.setIdentity();

    // JOINT_PELVIS_SPINE
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_SPINE], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_4), btScalar(M_PI_2));
    m_joints[JOINT_PELVIS_SPINE] = hingeC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_PELVIS_SPINE], true);

    // JOINT_SPINE_HEAD
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_HEAD], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, M_PI_2, 0.85f);
    m_joints[JOINT_SPINE_HEAD] = coneC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_SPINE_HEAD], true);

    // JOINT_LEFT_HIP
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_LEFT_UPPER_LEG], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, 0, 0.85f);
    m_joints[JOINT_LEFT_HIP] = coneC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_HIP], true);

    // JOINT_LEFT_KNEE
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_LEG], *m_bodies[BODYPART_LEFT_LOWER_LEG], localA, localB);
    hingeC->setLimit(btScalar(-2.f), btScalar(0));
    m_joints[JOINT_LEFT_KNEE] = hingeC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_KNEE], true);

    // JOINT_RIGHT_HIP
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_RIGHT_UPPER_LEG], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, 0, 0.85f);
    m_joints[JOINT_RIGHT_HIP] = coneC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_HIP], true);

    // JOINT_RIGHT_KNEE
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_LEG], *m_bodies[BODYPART_RIGHT_LOWER_LEG], localA, localB);
    hingeC->setLimit(btScalar(-2.f), btScalar(0));
    m_joints[JOINT_RIGHT_KNEE] = hingeC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_KNEE], true);

    // JOINT_LEFT_SHOULDER
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_LEFT_UPPER_ARM], localA, localB);
    coneC->setLimit(M_PI, M_PI, 0.1f, 0.85f);
    m_joints[JOINT_LEFT_SHOULDER] = coneC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_SHOULDER], true);

    // JOINT_LEFT_ELBOW
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_ARM], *m_bodies[BODYPART_LEFT_LOWER_ARM], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
    m_joints[JOINT_LEFT_ELBOW] = hingeC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_ELBOW], true);

    // JOINT_RIGHT_SHOULDER
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_RIGHT_UPPER_ARM], localA, localB);
    coneC->setLimit(M_PI, M_PI, 0.1f, 0.85f);
    m_joints[JOINT_RIGHT_SHOULDER] = coneC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_SHOULDER], true);

    // JOINT_RIGHT_ELBOW
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_ARM], *m_bodies[BODYPART_RIGHT_LOWER_ARM], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
    m_joints[JOINT_RIGHT_ELBOW] = hingeC;
    m_physicsWorld->m_dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_ELBOW], true);

    updateJointFrames();

    // joint targets
    m_fetalTargets[JOINT_PELVIS_SPINE].angle.x = -M_PI_2;
    m_fetalTargets[JOINT_PELVIS_SPINE].force = 500.f;
    m_fetalTargets[JOINT_PELVIS_SPINE].active = true;

    m_fetalTargets[JOINT_LEFT_KNEE].angle.x = -2.f;
    m_fetalTargets[JOINT_LEFT_KNEE].force = 60.f;
    m_fetalTargets[JOINT_LEFT_KNEE].active = true;

    m_fetalTargets[JOINT_RIGHT_KNEE].angle.x = -2.f;
    m_fetalTargets[JOINT_RIGHT_KNEE].force = 60.f;
    m_fetalTargets[JOINT_RIGHT_KNEE].active = true;

    m_fetalTargets[JOINT_LEFT_ELBOW].angle.x = -M_PI_2;
    m_fetalTargets[JOINT_LEFT_ELBOW].force = 60.f;
    m_fetalTargets[JOINT_LEFT_ELBOW].active = true;

    m_fetalTargets[JOINT_RIGHT_ELBOW].angle.x = -M_PI_2;
    m_fetalTargets[JOINT_RIGHT_ELBOW].force = 60.f;
    m_fetalTargets[JOINT_RIGHT_ELBOW].active = true;

    m_fetalTargets[JOINT_LEFT_HIP].angle = glm::vec4(0.707f, 0.f, 0.f, 0.707f);
    m_fetalTargets[JOINT_LEFT_HIP].force = 300.f;
    m_fetalTargets[JOINT_LEFT_HIP].active = true;

    m_fetalTargets[JOINT_RIGHT_HIP].angle = glm::vec4(0.707f, 0.f, 0.f, 0.707f);
    m_fetalTargets[JOINT_RIGHT_HIP].force = 300.f;
    m_fetalTargets[JOINT_RIGHT_HIP].active = true;

    m_fetalTargets[JOINT_LEFT_SHOULDER].angle = glm::vec4(0.466f, -0.372f, -0.802f, 0.024);
    m_fetalTargets[JOINT_LEFT_SHOULDER].force = 200.f;
    m_fetalTargets[JOINT_LEFT_SHOULDER].active = true;

    m_fetalTargets[JOINT_RIGHT_SHOULDER].angle = glm::vec4(-0.466f, -0.372f, -0.802f, 0.024);
    m_fetalTargets[JOINT_RIGHT_SHOULDER].force = 200.f;
    m_fetalTargets[JOINT_RIGHT_SHOULDER].active = true;

    m_fetalTargets[JOINT_SPINE_HEAD].angle = glm::vec4(-0.404f, 0.f, 0.f, 0.915);
    m_fetalTargets[JOINT_SPINE_HEAD].force = 150.f;
    m_fetalTargets[JOINT_SPINE_HEAD].active = true;
}

Ragdoll::~Ragdoll()
{
    int i;

    // Remove all constraints
    for (i = 0; i < JOINT_COUNT; ++i)
    {
        m_physicsWorld->m_dynamicsWorld->removeConstraint(m_joints[i]);
        delete m_joints[i];
        m_joints[i] = 0;
    }

    // Remove all bodies and shapes
    for (i = 0; i < BODYPART_COUNT; ++i)
    {
        m_physicsWorld->m_dynamicsWorld->removeRigidBody(m_bodies[i]);

        delete m_bodies[i]->getMotionState();

        delete m_bodies[i];
        m_bodies[i] = 0;
        delete m_shapes[i];
        m_shapes[i] = 0;
    }
}

AnimationNode Ragdoll::getNode(AssimpNodeData *node, std::string name, glm::mat4 parentTransform)
{
    if (node->name == name)
    {
        AnimationNode animNode;
        animNode.animNode = node;
        animNode.transform = parentTransform;
        return animNode;
    }

    for (int i = 0; i < node->children.size(); i++)
    {
        AnimationNode foundNode = getNode(node->children[i], name, parentTransform * node->transformation);
        if (foundNode.animNode && !foundNode.animNode->name.empty())
            return foundNode;
    }

    // TODO: should not end up here
    AnimationNode emptyNode;
    return emptyNode;
}

void Ragdoll::setupNode(RagdollNodeData &node, RagdollNodeData *parentNode, const std::string &name, int bodyPart, int joint)
{
    AnimationNode animationNode = getNode(m_animation->m_rootNode, name, glm::mat4(1.f));
    node.animNode = animationNode.animNode;
    node.transform = animationNode.transform;
    node.jointIndex = joint;
    node.boneIndex = m_animator->m_animations[0]->m_boneInfoMap[name].id;
    node.rigidBody = m_bodies[bodyPart];
    node.parentNode = parentNode;
    //
    btTransform transform;
    transform.setFromOpenGLMatrix((float *)&node.animNode->transformation);
    node.nodeRot = transform.getRotation();
    //
    transform.setFromOpenGLMatrix((float *)&node.transform);
    node.relativeNodeRot = transform.getRotation();
}

void Ragdoll::update(float deltaTime)
{
    if (m_status.prevState != m_status.state)
    {
        updateStateChange();
        m_status.prevState = m_status.state;
    }

    if (m_status.state == RagdollState::loose)
        return;

    JointTarget *targets;

    if (m_status.state == RagdollState::fetal)
        targets = m_fetalTargets;

    for (int i = 0; i < JOINT_COUNT; i++)
    {
        JointTarget &target = targets[i];

        if (!target.active)
            continue;

        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(m_joints[i]))
        {
            btConeTwistConstraint *constraint = (btConeTwistConstraint *)m_joints[i];
            constraint->setMotorTarget(btQuaternion(target.angle.x, target.angle.y, target.angle.z, target.angle.w));
            constraint->setMaxMotorImpulse(deltaTime * target.force);
        }
        else
        {
            btHingeConstraint *constraint = (btHingeConstraint *)m_joints[i];
            constraint->setMotorTarget(target.angle.x, deltaTime);
            constraint->setMaxMotorImpulse(deltaTime * target.force);
        }
    }
}

void Ragdoll::updateStateChange()
{
    bool enabled = m_status.state == RagdollState::fetal;

    for (int i = 0; i < JOINT_COUNT; i++)
    {
        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(m_joints[i]))
        {
            btConeTwistConstraint *constraint = (btConeTwistConstraint *)m_joints[i];
            constraint->setMotorTarget(btQuaternion::getIdentity());
            constraint->enableMotor(enabled);
            constraint->setMaxMotorImpulse(0.f);
        }
        else
        {
            btHingeConstraint *constraint = (btHingeConstraint *)m_joints[i];
            constraint->enableMotor(enabled);
            constraint->setMaxMotorImpulse(0.f);
        }
    }
}

void Ragdoll::changeState(RagdollState newState)
{
    m_status.prevState = m_status.state;
    m_status.state = newState;
}

void Ragdoll::updateJointSizes()
{
    updateJointSize(m_shapes[BODYPART_PELVIS], m_bodies[BODYPART_PELVIS], m_size.pelvisHeight);
    updateJointSize(m_shapes[BODYPART_SPINE], m_bodies[BODYPART_SPINE], m_size.spineHeight);
    updateJointSize(m_shapes[BODYPART_HEAD], m_bodies[BODYPART_HEAD], m_size.headHeight);
    updateJointSize(m_shapes[BODYPART_LEFT_UPPER_LEG], m_bodies[BODYPART_LEFT_UPPER_LEG], m_size.upperLegLength);
    updateJointSize(m_shapes[BODYPART_LEFT_LOWER_LEG], m_bodies[BODYPART_LEFT_LOWER_LEG], m_size.lowerLegLength);
    updateJointSize(m_shapes[BODYPART_RIGHT_UPPER_LEG], m_bodies[BODYPART_RIGHT_UPPER_LEG], m_size.upperLegLength);
    updateJointSize(m_shapes[BODYPART_RIGHT_LOWER_LEG], m_bodies[BODYPART_RIGHT_LOWER_LEG], m_size.lowerLegLength);
    updateJointSize(m_shapes[BODYPART_LEFT_UPPER_ARM], m_bodies[BODYPART_LEFT_UPPER_ARM], m_size.upperArmLength);
    updateJointSize(m_shapes[BODYPART_LEFT_LOWER_ARM], m_bodies[BODYPART_LEFT_LOWER_ARM], m_size.lowerArmLength);
    updateJointSize(m_shapes[BODYPART_RIGHT_UPPER_ARM], m_bodies[BODYPART_RIGHT_UPPER_ARM], m_size.upperArmLength);
    updateJointSize(m_shapes[BODYPART_RIGHT_LOWER_ARM], m_bodies[BODYPART_RIGHT_LOWER_ARM], m_size.lowerArmLength);
}

void Ragdoll::updateJointSize(btCapsuleShape *shape, btRigidBody *body, float size)
{
    btScalar radius = shape->getRadius();
    m_physicsWorld->m_dynamicsWorld->removeRigidBody(body);
    shape->setImplicitShapeDimensions(btVector3(radius, btScalar(size * 0.5) * m_scale, radius));
    m_physicsWorld->m_dynamicsWorld->addRigidBody(body);
}

// TODO: fully adapt m_size
void Ragdoll::updateJointFrames()
{
    btTransform localA, localB;
    btHingeConstraint *hingeC;
    btConeTwistConstraint *coneC;

    // JOINT_PELVIS_SPINE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.pelvisHeight / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.spineHeight / 2), btScalar(0.)));
    hingeC = (btHingeConstraint *)m_joints[JOINT_PELVIS_SPINE];
    hingeC->setFrames(localA, localB);

    // JOINT_SPINE_HEAD
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, M_PI_2);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.spineHeight), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.headHeight), btScalar(0.)));
    coneC = (btConeTwistConstraint *)m_joints[JOINT_SPINE_HEAD];
    coneC->setFrames(localA, localB);

    // JOINT_LEFT_HIP
    localA.setIdentity();
    localB.setIdentity();
    // .setEulerZYX(0, 0, -M_PI_4 * 5); why ???
    localA.getBasis().setEulerZYX(0, 0, -M_PI_4);
    localA.setOrigin(m_scale * btVector3(btScalar(0.09), btScalar(-m_size.pelvisHeight / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, -M_PI_4);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.upperLegLength / 2), btScalar(0.)));
    coneC = (btConeTwistConstraint *)m_joints[JOINT_LEFT_HIP];
    coneC->setFrames(localA, localB);

    // JOINT_LEFT_KNEE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.upperLegLength / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.lowerLegLength / 2), btScalar(0.)));
    hingeC = (btHingeConstraint *)m_joints[JOINT_LEFT_KNEE];
    hingeC->setFrames(localA, localB);

    // JOINT_RIGHT_HIP
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, M_PI_4);
    localA.setOrigin(m_scale * btVector3(btScalar(-0.09), btScalar(-m_size.pelvisHeight / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_4);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.upperLegLength / 2), btScalar(0.)));
    coneC = (btConeTwistConstraint *)m_joints[JOINT_RIGHT_HIP];
    coneC->setFrames(localA, localB);

    // JOINT_RIGHT_KNEE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.upperLegLength / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.lowerLegLength / 2), btScalar(0.)));
    hingeC = (btHingeConstraint *)m_joints[JOINT_RIGHT_KNEE];
    hingeC->setFrames(localA, localB);

    // JOINT_LEFT_SHOULDER
    localA.setIdentity();
    localB.setIdentity();
    localA.setRotation(btQuaternion(0.f, 0.707f, 0.707f, 0.f));
    localA.setOrigin(m_scale * btVector3(btScalar(m_size.shoulderOffset.x),
                                         btScalar(m_size.spineHeight / 2 + m_size.shoulderOffset.y),
                                         btScalar(m_size.shoulderOffset.z)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    // TODO: why to -Y direction instead of -X ?
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.upperArmLength / 2), btScalar(0.)));
    coneC = (btConeTwistConstraint *)m_joints[JOINT_LEFT_SHOULDER];
    coneC->setFrames(localA, localB);

    // JOINT_LEFT_ELBOW
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.upperArmLength / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.lowerArmLength / 2), btScalar(0.)));
    hingeC = (btHingeConstraint *)m_joints[JOINT_LEFT_ELBOW];
    hingeC->setFrames(localA, localB);

    // JOINT_RIGHT_SHOULDER
    localA.setIdentity();
    localB.setIdentity();
    localA.setRotation(btQuaternion(0.f, 0.707f, 0.f, 0.707f));
    localA.setOrigin(m_scale * btVector3(btScalar(-m_size.shoulderOffset.x),
                                         btScalar(m_size.spineHeight / 2 + m_size.shoulderOffset.y),
                                         btScalar(m_size.shoulderOffset.z)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.upperArmLength / 2), btScalar(0.)));
    coneC = (btConeTwistConstraint *)m_joints[JOINT_RIGHT_SHOULDER];
    coneC->setFrames(localA, localB);

    // JOINT_RIGHT_ELBOW
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(m_size.upperArmLength / 2), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-m_size.lowerArmLength / 2), btScalar(0.)));
    hingeC = (btHingeConstraint *)m_joints[JOINT_RIGHT_ELBOW];
    hingeC->setFrames(localA, localB);
}

void Ragdoll::resetTransforms(const btVector3 &offsetPosition, float angleY)
{
    resetTransforms(offsetPosition, btQuaternion(btVector3(btScalar(0.), btScalar(1.), btScalar(0.)), angleY));
}

// TODO: syncFromAnimation
// TODO: fully adapt m_size
void Ragdoll::resetTransforms(const btVector3 &offsetPosition, btQuaternion offsetRotation)
{
    btTransform offset;
    offset.setIdentity();
    offset.setOrigin(offsetPosition);
    offset.setRotation(offsetRotation);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(1.), btScalar(0.)));
    m_bodies[BODYPART_PELVIS]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(1.2), btScalar(0.)));
    m_bodies[BODYPART_SPINE]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(1.6), btScalar(0.)));
    m_bodies[BODYPART_HEAD]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.09), btScalar(0.65), btScalar(0.)));
    m_bodies[BODYPART_LEFT_UPPER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.09), btScalar(0.2), btScalar(0.)));
    m_bodies[BODYPART_LEFT_LOWER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.09), btScalar(0.65), btScalar(0.)));
    m_bodies[BODYPART_RIGHT_UPPER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.09), btScalar(0.2), btScalar(0.)));
    m_bodies[BODYPART_RIGHT_LOWER_LEG]->setWorldTransform(offset * transform);

    /* T pose
    {
        transform.setIdentity();
        transform.setOrigin(m_scale * btVector3(btScalar(-0.35), btScalar(1.45), btScalar(0.)));
        transform.getBasis().setEulerZYX(0, 0, M_PI_2);
        m_bodies[BODYPART_LEFT_UPPER_ARM]->setWorldTransform(offset * transform);

        transform.setIdentity();
        transform.setOrigin(m_scale * btVector3(btScalar(-0.7), btScalar(1.45), btScalar(0.)));
        transform.getBasis().setEulerZYX(0, 0, M_PI_2);
        m_bodies[BODYPART_LEFT_LOWER_ARM]->setWorldTransform(offset * transform);

        transform.setIdentity();
        transform.setOrigin(m_scale * btVector3(btScalar(0.35), btScalar(1.45), btScalar(0.)));
        transform.getBasis().setEulerZYX(0, 0, -M_PI_2);
        m_bodies[BODYPART_RIGHT_UPPER_ARM]->setWorldTransform(offset * transform);

        transform.setIdentity();
        transform.setOrigin(m_scale * btVector3(btScalar(0.7), btScalar(1.45), btScalar(0.)));
        transform.getBasis().setEulerZYX(0, 0, -M_PI_2);
        m_bodies[BODYPART_RIGHT_LOWER_ARM]->setWorldTransform(offset * transform);
    } */

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.2), btScalar(1.25), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, 0, -M_PI + M_1_PI);
    m_bodies[BODYPART_LEFT_UPPER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.3), btScalar(1.), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, M_PI_2, -M_PI + M_1_PI);
    m_bodies[BODYPART_LEFT_LOWER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.2), btScalar(1.25), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, 0, M_PI - M_1_PI);
    m_bodies[BODYPART_RIGHT_UPPER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.3), btScalar(1.), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, -M_PI_2, M_PI - M_1_PI);
    m_bodies[BODYPART_RIGHT_LOWER_ARM]->setWorldTransform(offset * transform);
}

void Ragdoll::freezeBodies()
{
    for (int i = 0; i < BODYPART_COUNT; ++i)
    {
        m_bodies[i]->setLinearFactor(btVector3(0.0f, 0.0f, 0.0f));
        m_bodies[i]->setActivationState(0);
        m_bodies[i]->setCollisionFlags(m_bodies[i]->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

void Ragdoll::unFreezeBodies()
{
    for (int i = 0; i < BODYPART_COUNT; ++i)
    {
        // m_bodies[i]->setLinearFactor(btVector3(0.1f, 0.1f, 0.1f));
        m_bodies[i]->setLinearFactor(btVector3(1.0f, 1.0f, 1.0f));
        m_bodies[i]->setActivationState(1);
        m_bodies[i]->setCollisionFlags(m_bodies[i]->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

void Ragdoll::syncToAnimation(glm::vec3 &position)
{
    const btQuaternion identity(0, 0, 0, 1);

    // parent
    btTransform transform;
    nodePelvis.rigidBody->getMotionState()->getWorldTransform(transform);
    position = BulletGLM::getGLMVec3(transform.getOrigin());
    position += m_modelOffset;

    syncNodeToAnimation(&nodePelvis, m_pelvisOffset);
    syncNodeToAnimation(&nodeSpine, m_spineOffset);
    syncNodeToAnimation(&nodeHead, m_headOffset);

    syncNodeToAnimation(&nodeLeftUpLeg, m_leftLegOffset, true);
    syncNodeToAnimation(&nodeLeftLeg, identity);
    syncNodeToAnimation(&nodeRightUpLeg, m_rightLegOffset, true);
    syncNodeToAnimation(&nodeRightLeg, identity);

    syncNodeToAnimation(&nodeLeftArm, m_leftArmOffset, true);
    syncNodeToAnimation(&nodeLeftForeArm, m_leftForeArmOffset, true);
    syncNodeToAnimation(&nodeRightArm, m_rightArmOffset, true);
    syncNodeToAnimation(&nodeRightForeArm, m_rightForeArmOffset, true);
}

// TODO: matrix3x3?
void Ragdoll::syncNodeToAnimation(RagdollNodeData *node, btQuaternion offset, bool flipVertical)
{
    btQuaternion boneRot = node->rigidBody->getOrientation();
    node->boneRot = boneRot;

    btQuaternion parentBoneRot = node->parentNode->boneRot;
    btQuaternion parentRelativeNodeRot = node->parentNode->relativeNodeRot;

    // orient = diff * parent
    btQuaternion diffOrientation = parentBoneRot.inverse() * boneRot;
    btQuaternion diffRelativeNodeRot = parentRelativeNodeRot.inverse() * node->relativeNodeRot;

    if (flipVertical)
    {
        diffOrientation = btQuaternion(-diffOrientation.getX(),
                                       -diffOrientation.getY(),
                                       diffOrientation.getZ(),
                                       diffOrientation.getW());
    }

    btQuaternion boneOrientation = offset * node->nodeRot * node->relativeNodeRot * diffOrientation * diffRelativeNodeRot;

    Bone *bone = m_animation->getBone(node->animNode->name);
    bone->m_rotations[0].value = BulletGLM::getGLMQuat(boneOrientation);
    bone->updatePose();
}

void Ragdoll::syncFromAnimation(glm::mat4 characterModel)
{
    const glm::quat idendity = glm::quat(1.f, 0.f, 0.f, 0.f);

    syncNodeFromAnimation(nodePelvis, characterModel, idendity, false);
    syncNodeFromAnimation(nodeSpine, characterModel, idendity, false);
    syncNodeFromAnimation(nodeHead, characterModel, idendity, false);

    syncNodeFromAnimation(nodeLeftUpLeg, characterModel, m_legOffset);
    syncNodeFromAnimation(nodeLeftLeg, characterModel, m_legOffset);
    syncNodeFromAnimation(nodeRightUpLeg, characterModel, m_legOffset);
    syncNodeFromAnimation(nodeRightLeg, characterModel, m_legOffset);

    syncNodeFromAnimation(nodeLeftArm, characterModel, idendity);
    syncNodeFromAnimation(nodeLeftForeArm, characterModel, idendity);
    syncNodeFromAnimation(nodeRightArm, characterModel, idendity);
    syncNodeFromAnimation(nodeRightForeArm, characterModel, idendity);
}

void Ragdoll::syncNodeFromAnimation(const RagdollNodeData &node, glm::mat4 characterModel, glm::quat offset, bool offsetSize)
{
    glm::mat4 armatureOffset(1.f);
    // scale offset
    armatureOffset = glm::scale(armatureOffset, m_armatureScale);
    // translate offset
    if (offsetSize)
    {
        btCapsuleShape *shape = (btCapsuleShape *)node.rigidBody->getCollisionShape();
        float halfHeight = shape->getHalfHeight();
        armatureOffset = glm::translate(armatureOffset, glm::vec3(0.f, halfHeight, 0.f));
    }

    glm::mat4 boneModel = characterModel * m_animator->m_globalMatrices[node.boneIndex] * armatureOffset * glm::mat4_cast(offset);

    btTransform transform;
    transform.setFromOpenGLMatrix((float *)&boneModel);
    node.rigidBody->setWorldTransform(transform);
}
