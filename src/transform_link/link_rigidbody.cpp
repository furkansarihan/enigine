#include "link_rigidbody.h"

TransformLinkRigidBody::TransformLinkRigidBody(btRigidBody *rigidbody, eTransform offset)
    : m_rigidbody(rigidbody),
      m_offset(offset)
{
}

TransformLinkRigidBody::~TransformLinkRigidBody()
{
}

glm::mat4 TransformLinkRigidBody::getModelMatrix()
{
    btTransform transform;
    m_rigidbody->getMotionState()->getWorldTransform(transform);

    glm::mat4 model;
    transform.getOpenGLMatrix((btScalar *)&model);

    return model * m_offset.getModelMatrix();
}
