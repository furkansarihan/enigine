#ifndef transform_hpp
#define transform_hpp

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform
{
public:
    Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
    Transform();
    ~Transform();

    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_model;

    void updateModelMatrix();
};

#endif /* transform_hpp */
