#include "link_door.h"

TransformLinkDoor::TransformLinkDoor(Vehicle *vehicle, int doorIndex, eTransform offsetA, eTransform offsetD)
    : m_vehicle(vehicle),
      m_doorIndex(doorIndex),
      m_offsetActive(offsetA),
      m_offsetDeactive(offsetD)
{
}

TransformLinkDoor::~TransformLinkDoor()
{
}

glm::mat4 TransformLinkDoor::getModelMatrix()
{
    if (m_vehicle->m_doors[m_doorIndex].hingeState == HingeState::deactive)
    {
        btTransform transform;
        m_vehicle->m_carChassis->getMotionState()->getWorldTransform(transform);

        glm::mat4 model;
        transform.getOpenGLMatrix((btScalar *)&model);

        return model * m_offsetDeactive.getModelMatrix();
    }
    else
    {
        // TODO: remove this workaround
        btTransform transform;
        if (m_vehicle->m_speed > 10.f)
            m_vehicle->m_doors[m_doorIndex].body->getMotionState()->getWorldTransform(transform);
        else
            transform = m_vehicle->m_doors[m_doorIndex].body->getWorldTransform();

        glm::mat4 model;
        transform.getOpenGLMatrix((btScalar *)&model);

        return model * m_offsetActive.getModelMatrix();
    }
}
