
#include "transform.h"

eTransform::eTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
    : m_position(position),
      m_rotation(rotation),
      m_scale(scale)
{
    updateModelMatrix();
}

eTransform::eTransform()
    : m_position(glm::vec3(0.f)),
      m_rotation(glm::quat(1.f, 0.f, 0.f, 0.f)),
      m_scale(glm::vec3(1.f))
{
    updateModelMatrix();
}

eTransform::~eTransform()
{
}

void eTransform::setPosition(glm::vec3 position)
{
    m_position = position;
    updateModelMatrix();
}

void eTransform::setRotation(glm::quat rotation)
{
    m_rotation = rotation;
    updateModelMatrix();
}

void eTransform::setScale(glm::vec3 scale)
{
    m_scale = scale;
    updateModelMatrix();
}

void eTransform::setTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
    m_position = position;
    m_rotation = rotation;
    m_scale = scale;
    updateModelMatrix();
}

// TODO: remove?
void eTransform::setModelMatrix(glm::mat4 model)
{
    m_model = model;
}

void eTransform::updateModelMatrix()
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, m_position);
    model = model * glm::mat4_cast(m_rotation);
    model = glm::scale(model, m_scale);
    m_model = model;
}