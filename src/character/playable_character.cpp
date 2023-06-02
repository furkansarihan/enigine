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
    {
        m_followOffsetTarget = m_controller->m_aimLocked ? m_followOffsetAim : m_followOffsetNormal;
        m_followOffsetTarget.y += m_controller->m_onGround ? m_controller->m_worldElevation : m_position.y;

        m_followOffset = glm::mix(m_followOffset, m_followOffsetTarget, m_followOffsetFactor);

        glm::vec3 worldRef(m_position.x, m_followOffset.y, m_position.z);
        glm::vec3 offsetX = m_followCamera->right * glm::vec3(m_followOffset.x);
        glm::vec3 offsetZ = m_followCamera->front * glm::vec3(m_followOffset.z);
        m_followCamera->position = worldRef + offsetX + offsetZ;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        // 1 hit per second
        float now = (float)glfwGetTime();
        if (now - m_lastFire > 1.0f)
        {
            m_lastFire = now;
            fireWeapon();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        for (int i = 0; i < m_npcList.size(); i++)
            m_npcList[i]->resetRagdoll();
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        m_controller->m_aimLocked = false;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        m_controller->m_aimLocked = true;
    }
}

void PCharacter::fireWeapon()
{
    // shoot ray
    btVector3 from = BulletGLM::getBulletVec3(m_position + m_followOffset);
    btVector3 to = BulletGLM::getBulletVec3(m_followCamera->front * 20000.f);
    btCollisionWorld::ClosestRayResultCallback callback(from, to);
    m_controller->m_dynamicsWorld->rayTest(from, to, callback);

    // check if it's collide any NPC
    if (callback.hasHit())
    {
        // std::cout << "PCharacter::fireWeapon: hit" << std::endl;
        const btRigidBody *rb = btRigidBody::upcast(callback.m_collisionObject);
        // TODO: O(1) lookup
        for (int i = 0; i < m_npcList.size(); i++)
        {
            if (m_npcList[i]->m_rigidbody == rb)
                m_npcList[i]->activateRagdoll(m_followCamera->front, m_impulseStrength);
        }
    }
    // else
    //     std::cout << "PCharacter::fireWeapon: no hit" << std::endl;
}
