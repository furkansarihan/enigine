#ifndef link_wheel_hpp
#define link_wheel_hpp

#include <iostream>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../transform_link/transform_link.h"
#include "../transform/transform.h"

class TransformLinkWheel : public TransformLink
{
public:
    TransformLinkWheel(btRaycastVehicle *vehicle, int wheelIndex, eTransform offset);
    ~TransformLinkWheel();

    btRaycastVehicle *m_vehicle;
    int m_wheelIndex;
    eTransform m_offset;

    glm::mat4 getModelMatrix() override;
};

#endif /* link_wheel_hpp */
