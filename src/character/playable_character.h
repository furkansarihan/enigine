#ifndef playable_character_hpp
#define playable_character_hpp

#include "character.h"

#include <GLFW/glfw3.h>

class PCharacter : public Character
{
public:
    PCharacter(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~PCharacter();

    bool m_controlCharacter = true;
    bool m_followCharacter = true;
    float m_followDistance = 8.0f;
    glm::vec3 m_followOffset = glm::vec3(0.0f, 2.5f, 0.0f);

    void update(GLFWwindow *window, float deltaTime);
};

#endif /* playable_character_hpp */
