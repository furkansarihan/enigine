#include "playable_character.h"

PCharacter::PCharacter(ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : Character(m_shaderManager, physicsWorld, followCamera)
{
}

PCharacter::~PCharacter()
{
}

void PCharacter::update(GLFWwindow *window, float deltaTime)
{
    Character::update(deltaTime);

    if (m_controlCharacter)
        m_controller->recieveInput(window, deltaTime);
    if (m_followCharacter)
            m_followCamera->position = m_position - m_followCamera->front * glm::vec3(m_followDistance) + m_followOffset;


    m_controller->updateRagdollAction(m_ragdoll, m_position, m_rotation, window, deltaTime);
}
