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

    glm::vec3 getPosition() const { return m_position; }
    glm::quat getRotation() const { return m_rotation; }
    glm::vec3 getScale() const { return m_scale; }
    glm::mat4 getModelMatrix() const { return m_model; }

    void setPosition(glm::vec3 position);
    void setRotation(glm::quat rotation);
    void setScale(glm::vec3 scale);
    void setTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
    void setModelMatrix(glm::mat4 model);

private:
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_model;

    void updateModelMatrix();
};

#endif /* transform_hpp */
