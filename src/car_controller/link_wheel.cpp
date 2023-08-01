#include "link_wheel.h"

TransformLinkWheel::TransformLinkWheel(btRaycastVehicle *vehicle, int wheelIndex, eTransform offset)
    : m_vehicle(vehicle),
      m_wheelIndex(wheelIndex),
      m_offset(offset)
{
}

TransformLinkWheel::~TransformLinkWheel()
{
}

glm::mat4 TransformLinkWheel::getModelMatrix()
{
    glm::mat4 model;
    m_vehicle->getWheelInfo(m_wheelIndex).m_worldTransform.getOpenGLMatrix((btScalar *)&model);

    return model * m_offset.getModelMatrix();
}
