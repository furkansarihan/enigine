#ifndef link_door_hpp
#define link_door_hpp

#include <iostream>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../transform_link/transform_link.h"
#include "../transform/transform.h"
#include "../vehicle/vehicle.h"

class TransformLinkDoor : public TransformLink
{
public:
    TransformLinkDoor(Vehicle *vehicle, int doorIndex, eTransform offsetA, eTransform offsetD);
    ~TransformLinkDoor();

    Vehicle *m_vehicle;
    int m_doorIndex;
    eTransform m_offsetActive;
    eTransform m_offsetDeactive;

    glm::mat4 getModelMatrix() override;
};

#endif /* link_door_hpp */
