#ifndef link_rigidbody_hpp
#define link_rigidbody_hpp

#include <iostream>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "transform_link.h"
#include "../transform/transform.h"

class TransformLinkRigidBody : public TransformLink
{
public:
    TransformLinkRigidBody(btRigidBody *rigidbody, eTransform offset);
    ~TransformLinkRigidBody();

    btRigidBody *m_rigidbody;
    eTransform m_offset;

    glm::mat4 getModelMatrix() override;
};

#endif /* link_rigidbody_hpp */
