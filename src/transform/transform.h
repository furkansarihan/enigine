#ifndef transform_hpp
#define transform_hpp

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class eTransform
{
public:
    eTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
    eTransform();
    ~eTransform();

    const glm::vec3 &getPosition() const
    {
        return m_position;
    }
    const glm::quat &getRotation() const
    {
        return m_rotation;
    }
    const glm::vec3 &getScale() const
    {
        return m_scale;
    }
    const glm::mat4 &getModelMatrix() const
    {
        return m_model;
    }

    void setPosition(const glm::vec3& position);
    void setRotation(const glm::quat& rotation);
    void setScale(const glm::vec3& scale);
    void setTransform(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale);
    void setModelMatrix(const glm::mat4 &model);

private:
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_model;

    void updateModelMatrix();
};

#endif /* transform_hpp */
