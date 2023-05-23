#include "ragdoll.h"

AssimpNodeData &getNode(AssimpNodeData &node, std::string name);

Ragdoll::Ragdoll(PhysicsWorld *physicsWorld, Animation *animation, const btVector3 &positionOffset, btScalar scale)
    : m_physicsWorld(physicsWorld),
      m_animation(animation),
      m_scale(scale)
{
    m_shapes[BODYPART_PELVIS] = new btCapsuleShape(btScalar(0.15) * m_scale, btScalar(0.20) * m_scale);
    m_shapes[BODYPART_SPINE] = new btCapsuleShape(btScalar(0.15) * m_scale, btScalar(0.28) * m_scale);
    m_shapes[BODYPART_HEAD] = new btCapsuleShape(btScalar(0.10) * m_scale, btScalar(0.05) * m_scale);
    m_shapes[BODYPART_LEFT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07) * m_scale, btScalar(0.45) * m_scale);
    m_shapes[BODYPART_LEFT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(0.37) * m_scale);
    m_shapes[BODYPART_RIGHT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07) * m_scale, btScalar(0.45) * m_scale);
    m_shapes[BODYPART_RIGHT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(0.37) * m_scale);
    m_shapes[BODYPART_LEFT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(0.33) * m_scale);
    m_shapes[BODYPART_LEFT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04) * m_scale, btScalar(0.25) * m_scale);
    m_shapes[BODYPART_RIGHT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05) * m_scale, btScalar(0.33) * m_scale);
    m_shapes[BODYPART_RIGHT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04) * m_scale, btScalar(0.25) * m_scale);

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

    // save rigidbodies to nodes
    AssimpNodeData &nodePelvis = getNode(m_animation->m_RootNode, "mixamorig:Hips");
    nodePelvis.rigidBody = m_bodies[BODYPART_PELVIS];
    AssimpNodeData &nodeSpine = getNode(m_animation->m_RootNode, "mixamorig:Spine1");
    nodeSpine.rigidBody = m_bodies[BODYPART_SPINE];
    AssimpNodeData &nodeHead = getNode(m_animation->m_RootNode, "mixamorig:Head");
    nodeHead.rigidBody = m_bodies[BODYPART_HEAD];
    AssimpNodeData &nodeLeftUpLeg = getNode(m_animation->m_RootNode, "mixamorig:LeftUpLeg");
    nodeLeftUpLeg.rigidBody = m_bodies[BODYPART_LEFT_UPPER_LEG];
    AssimpNodeData &nodeLeftLeg = getNode(m_animation->m_RootNode, "mixamorig:LeftLeg");
    nodeLeftLeg.rigidBody = m_bodies[BODYPART_LEFT_LOWER_LEG];
    AssimpNodeData &nodeRightUpLeg = getNode(m_animation->m_RootNode, "mixamorig:RightUpLeg");
    nodeRightUpLeg.rigidBody = m_bodies[BODYPART_RIGHT_UPPER_LEG];
    AssimpNodeData &nodeRightLeg = getNode(m_animation->m_RootNode, "mixamorig:RightLeg");
    nodeRightLeg.rigidBody = m_bodies[BODYPART_RIGHT_LOWER_LEG];
    AssimpNodeData &nodeRightArm = getNode(m_animation->m_RootNode, "mixamorig:RightArm");
    nodeRightArm.rigidBody = m_bodies[BODYPART_RIGHT_UPPER_ARM];
    AssimpNodeData &nodeRightForeArm = getNode(m_animation->m_RootNode, "mixamorig:RightForeArm");
    nodeRightForeArm.rigidBody = m_bodies[BODYPART_RIGHT_LOWER_ARM];
    AssimpNodeData &nodeLeftArm = getNode(m_animation->m_RootNode, "mixamorig:LeftArm");
    nodeLeftArm.rigidBody = m_bodies[BODYPART_LEFT_UPPER_ARM];
    AssimpNodeData &nodeLeftForeArm = getNode(m_animation->m_RootNode, "mixamorig:LeftForeArm");
    nodeLeftForeArm.rigidBody = m_bodies[BODYPART_LEFT_LOWER_ARM];

    // TODO: fix with joint postitions
    nodeLeftUpLeg.rigidBody = m_bodies[BODYPART_RIGHT_UPPER_LEG];
    nodeLeftLeg.rigidBody = m_bodies[BODYPART_RIGHT_LOWER_LEG];
    nodeRightUpLeg.rigidBody = m_bodies[BODYPART_LEFT_UPPER_LEG];
    nodeRightLeg.rigidBody = m_bodies[BODYPART_LEFT_LOWER_LEG];
    nodeLeftArm.rigidBody = m_bodies[BODYPART_RIGHT_UPPER_ARM];
    nodeLeftForeArm.rigidBody = m_bodies[BODYPART_RIGHT_LOWER_ARM];
    nodeRightArm.rigidBody = m_bodies[BODYPART_LEFT_UPPER_ARM];
    nodeRightForeArm.rigidBody = m_bodies[BODYPART_LEFT_LOWER_ARM];

    // constraints
    btHingeConstraint *hingeC;
    btConeTwistConstraint *coneC;

    btTransform localA, localB;

    // JOINT_PELVIS_SPINE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.15), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.15), btScalar(0.)));
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_SPINE], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_4), btScalar(M_PI_2));
    m_joints[JOINT_PELVIS_SPINE] = hingeC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_PELVIS_SPINE], true);
    nodeSpine.index = JOINT_PELVIS_SPINE;

    // JOINT_SPINE_HEAD
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, M_PI_2);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.30), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_HEAD], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, M_PI_2);
    m_joints[JOINT_SPINE_HEAD] = coneC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_SPINE_HEAD], true);
    nodeHead.index = JOINT_SPINE_HEAD;

    // JOINT_LEFT_HIP
    localA.setIdentity();
    localB.setIdentity();
    // .setEulerZYX(0, 0, -M_PI_4 * 5); why ???
    localA.getBasis().setEulerZYX(0, 0, -M_PI_4);
    localA.setOrigin(m_scale * btVector3(btScalar(-0.09), btScalar(-0.10), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, -M_PI_4);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_LEFT_UPPER_LEG], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, 0);
    m_joints[JOINT_LEFT_HIP] = coneC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_HIP], true);
    nodeLeftUpLeg.index = JOINT_LEFT_HIP;

    // JOINT_LEFT_KNEE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, -M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, -M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_LEG], *m_bodies[BODYPART_LEFT_LOWER_LEG], localA, localB);
    hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
    m_joints[JOINT_LEFT_KNEE] = hingeC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_KNEE], true);
    nodeLeftLeg.index = JOINT_LEFT_KNEE;

    // JOINT_RIGHT_HIP
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, M_PI_4);
    localA.setOrigin(m_scale * btVector3(btScalar(0.09), btScalar(-0.10), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_4);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_RIGHT_UPPER_LEG], localA, localB);
    coneC->setLimit(M_PI_4, M_PI_4, 0);
    m_joints[JOINT_RIGHT_HIP] = coneC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_HIP], true);
    nodeRightUpLeg.index = JOINT_RIGHT_HIP;

    // JOINT_RIGHT_KNEE
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, -M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, -M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_LEG], *m_bodies[BODYPART_RIGHT_LOWER_LEG], localA, localB);
    hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
    m_joints[JOINT_RIGHT_KNEE] = hingeC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_KNEE], true);
    nodeRightLeg.index = JOINT_RIGHT_KNEE;

    // JOINT_LEFT_SHOULDER
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, M_PI);
    localA.setOrigin(m_scale * btVector3(btScalar(-0.2), btScalar(0.15), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    // TODO: why to -Y direction instead of -X ?
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_LEFT_UPPER_ARM], localA, localB);
    coneC->setLimit(M_PI_2, M_PI_2, 0);
    m_joints[JOINT_LEFT_SHOULDER] = coneC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_SHOULDER], true);
    nodeLeftArm.index = JOINT_LEFT_SHOULDER;

    // JOINT_LEFT_ELBOW
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_ARM], *m_bodies[BODYPART_LEFT_LOWER_ARM], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
    m_joints[JOINT_LEFT_ELBOW] = hingeC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_LEFT_ELBOW], true);
    nodeLeftForeArm.index = JOINT_LEFT_ELBOW;

    // JOINT_RIGHT_SHOULDER
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, 0, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.2), btScalar(0.15), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, 0, M_PI_2);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
    coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_RIGHT_UPPER_ARM], localA, localB);
    coneC->setLimit(M_PI_2, M_PI_2, 0);
    m_joints[JOINT_RIGHT_SHOULDER] = coneC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_SHOULDER], true);
    nodeRightArm.index = JOINT_RIGHT_SHOULDER;

    // JOINT_RIGHT_ELBOW
    localA.setIdentity();
    localB.setIdentity();
    localA.getBasis().setEulerZYX(0, M_PI_2, 0);
    localA.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
    localB.getBasis().setEulerZYX(0, M_PI_2, 0);
    localB.setOrigin(m_scale * btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
    hingeC = new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_ARM], *m_bodies[BODYPART_RIGHT_LOWER_ARM], localA, localB);
    hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
    m_joints[JOINT_RIGHT_ELBOW] = hingeC;
    m_physicsWorld->dynamicsWorld->addConstraint(m_joints[JOINT_RIGHT_ELBOW], true);
    nodeRightForeArm.index = JOINT_RIGHT_ELBOW;
}

Ragdoll::~Ragdoll()
{
    int i;

    // Remove all constraints
    for (i = 0; i < JOINT_COUNT; ++i)
    {
        m_physicsWorld->dynamicsWorld->removeConstraint(m_joints[i]);
        delete m_joints[i];
        m_joints[i] = 0;
    }

    // Remove all bodies and shapes
    for (i = 0; i < BODYPART_COUNT; ++i)
    {
        m_physicsWorld->dynamicsWorld->removeRigidBody(m_bodies[i]);

        delete m_bodies[i]->getMotionState();

        delete m_bodies[i];
        m_bodies[i] = 0;
        delete m_shapes[i];
        m_shapes[i] = 0;
    }
}

AssimpNodeData &getNode(AssimpNodeData &node, std::string name)
{
    if (node.name == name)
        return node;

    for (int i = 0; i < node.childrenCount; i++)
    {
        AssimpNodeData &foundNode = getNode(node.children[i], name);
        if (!foundNode.name.empty())
            return foundNode;
    }

    AssimpNodeData emptyNode;
    return emptyNode;
}

// TODO: initial orientation around Y axis
void Ragdoll::resetTransforms(const btVector3 &positionOffset, float angleY)
{
    btTransform offset;
    offset.setIdentity();
    offset.setOrigin(positionOffset);
    offset.setRotation(btQuaternion(btVector3(btScalar(0.), btScalar(1.), btScalar(0.)), angleY));

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
    transform.setOrigin(m_scale * btVector3(btScalar(-0.18), btScalar(0.65), btScalar(0.)));
    m_bodies[BODYPART_LEFT_UPPER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.18), btScalar(0.2), btScalar(0.)));
    m_bodies[BODYPART_LEFT_LOWER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.18), btScalar(0.65), btScalar(0.)));
    m_bodies[BODYPART_RIGHT_UPPER_LEG]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.18), btScalar(0.2), btScalar(0.)));
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
    transform.setOrigin(m_scale * btVector3(btScalar(-0.2), btScalar(1.25), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, 0, M_PI - M_1_PI);
    m_bodies[BODYPART_LEFT_UPPER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(-0.3), btScalar(1.), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, -M_PI_2, M_PI - M_1_PI);
    m_bodies[BODYPART_LEFT_LOWER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.2), btScalar(1.25), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, 0, -M_PI + M_1_PI);
    m_bodies[BODYPART_RIGHT_UPPER_ARM]->setWorldTransform(offset * transform);

    transform.setIdentity();
    transform.setOrigin(m_scale * btVector3(btScalar(0.3), btScalar(1.), btScalar(0.)));
    transform.getBasis().setEulerZYX(0, M_PI_2, -M_PI + M_1_PI);
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
        m_bodies[i]->setLinearFactor(btVector3(1.0f, 1.0f, 1.0f));
        m_bodies[i]->setActivationState(1);
        m_bodies[i]->setCollisionFlags(m_bodies[i]->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
}

void Ragdoll::syncToAnimation(glm::vec3 &position)
{
    btQuaternion identity(0, 0, 0, 1);
    syncNodeToAnimation(m_animation->m_RootNode, identity, identity, position, true);
}

void Ragdoll::syncNodeToAnimation(AssimpNodeData node, btQuaternion skipNodeOrientation, btQuaternion parentOrientation, glm::vec3 &position, bool fromParent)
{
    // end of tree
    if (node.childrenCount == 0)
        return;

    btTransform transform;
    transform.setFromOpenGLMatrix((float *)&node.transformation);
    btQuaternion nodeOrientation = transform.getRotation();

    // continue if the node is not a Bone or empty rigidbody
    auto boneInfoMap = m_animation->m_BoneInfoMap;
    if (boneInfoMap.find(node.name) == boneInfoMap.end() || node.rigidBody == nullptr)
    {
        skipNodeOrientation = nodeOrientation * skipNodeOrientation;

        for (int i = 0; i < node.childrenCount; i++)
            syncNodeToAnimation(node.children[i], skipNodeOrientation, parentOrientation, position, fromParent);

        return;
    }

    // TODO: rigidBody is nullptr?
    btQuaternion orientation = node.rigidBody->getOrientation();
    btQuaternion boneOrientation;

    if (fromParent)
    {
        position = BulletGLM::getGLMVec3(node.rigidBody->getWorldTransform().getOrigin());
        position += m_modelOffset;
    }

    // joint orientation
    // orient = diff * parent
    btQuaternion diffOrientation = parentOrientation.inverse() * orientation;
    diffs[node.index] = diffOrientation;
    skips[node.index] = skipNodeOrientation;

    if (node.name == "mixamorig:RightUpLeg" ||
        node.name == "mixamorig:LeftUpLeg" ||
        node.name == "mixamorig:RightLeg" ||
        node.name == "mixamorig:LeftLeg")
        diffOrientation = btQuaternion(-diffOrientation.getX(),
                                       -diffOrientation.getY(),
                                       diffOrientation.getZ(),
                                       diffOrientation.getW());

    //
    boneOrientation = skipNodeOrientation * nodeOrientation * diffOrientation;

    // TODO: correct way
    if (node.name == "mixamorig:RightArm")
    {
        btQuaternion first(-0.5, 0.5, -0.5, -0.5);
        btQuaternion last(-0.5, 0.5, -0.5, 0.5);
        // last = first * diff
        btQuaternion diff = first.inverse() * last;

        boneOrientation = diff * boneOrientation;
    }
    if (node.name == "mixamorig:LeftArm")
    {
        btQuaternion first(0.5, 0.5, -0.5, 0.5);
        btQuaternion last(-0.5, -0.5, 0.5, 0.5);
        // last = first * diff
        btQuaternion diff = first.inverse() * last;

        boneOrientation = diff * boneOrientation;
    }

    // TODO: bone is nullptr?
    Bone *bone = m_animation->getBone(node.name);
    bone->m_Rotations[0].orientation = BulletGLM::getGLMQuat(boneOrientation);
    bone->updateInternal(0.0f);

    bones[node.index] = bone;

    orients[node.index] = orientation;
    parentOrients[node.index] = parentOrientation;
    nodeOrients[node.index] = nodeOrientation;
    boneOrients[node.index] = boneOrientation;

    for (int i = 0; i < node.childrenCount; i++)
        syncNodeToAnimation(node.children[i], btQuaternion(0, 0, 0, 1), orientation, position, false);
}
