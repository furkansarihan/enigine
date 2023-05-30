#ifndef playable_character_hpp
#define playable_character_hpp

#include "character.h"

class PCharacter : public Character
{
public:
    PCharacter(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~PCharacter();

    std::vector<Character *> m_npcList;

    bool m_firstPerson = false;
    bool m_controlCharacter = true;
    bool m_followCharacter = true;
    float m_followDistance = 8.0f;
    glm::vec3 m_followOffset = glm::vec3(0.0f, 2.5f, 0.0f);

    float m_followHeight = 0.f;
    float m_followHeightTarget = 0.f;
    float m_followHeightFactor = 0.1f;

    float m_lastFire;

    void update(GLFWwindow *window, float deltaTime);
    void fireWeapon();
};

#endif /* playable_character_hpp */
