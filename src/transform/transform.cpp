
#include "transform.h"

Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : m_position(position),
      m_rotation(rotation),
      m_scale(scale)
{
}

Transform::Transform()
    : m_position(glm::vec3(0.f)),
      m_rotation(glm::quat(1.f, 0.f, 0.f, 0.f)),
      m_scale(glm::vec3(1.f))
{
}

Transform::~Transform()
{
}

void Transform::updateModelMatrix()
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, m_position);
    model = model * glm::mat4_cast(m_rotation);
    model = glm::scale(model, m_scale);
    m_model = model;
}