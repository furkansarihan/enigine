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

    glm::vec3 m_followOffsetAim = glm::vec3(-1.2f, 3.4f, -2.3f);
    glm::vec3 m_followOffsetNormal = glm::vec3(0.0f, 3.f, -8.f);
    glm::vec3 m_followOffset = glm::vec3(0.0f, 3.f, -8.f);
    glm::vec3 m_followOffsetTarget = glm::vec3(0.0f, 3.f, -8.f);
    float m_followOffsetFactor = 0.1f;

    // glm::vec3 m_pistolOffset = glm::vec3(0.f, 14.6f, 2.f);
    // glm::quat m_pistolOrientation = glm::quat(0.354f, 0.43f, -0.561f, -0.612f); // 90, 7.5, -112
    // float m_pistolScale = 3.2f;
    // TODO: bound to pistol model
    glm::vec3 m_pistolOffset = glm::vec3(1.1f, 16.2f, 3.4f);
    glm::quat m_pistolOrientation = glm::quat(0.563f, -0.432f, -0.447f, -0.545f); // 0, -76, -88
    float m_pistolScale = 100.f;

    float m_lastFire;
    float m_fireLimit = 0.5f;
    float m_fireAnimStartTime = 1.f;
    int m_fireAnimTimeMilli = 250;

    void update(GLFWwindow *window, float deltaTime);
    void fireWeapon();
    void shootRay();
};

#endif /* playable_character_hpp */
