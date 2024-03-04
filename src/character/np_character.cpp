#include "np_character.h"

NPCharacter::NPCharacter(RenderManager *renderManager, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : Character(renderManager, resourceManager, physicsWorld, followCamera)
{
}

NPCharacter::~NPCharacter()
{
}

void NPCharacter::update(float deltaTime)
{
    Character::update(deltaTime);
    // TODO: convert task
    // avoidAim(deltaTime);
}

void NPCharacter::avoidAim(float deltaTime)
{
    bool intersect;
    for (int i = 0; i < m_avoidAimList.size(); i++)
    {
        Character *character = m_avoidAimList[i];

        btVector3 aabbMin, aabbMax;
        m_rigidbody->getAabb(aabbMin, aabbMax);
        aabbMin = aabbMin - btVector3(1.0f, 1.0f, 1.0f);
        aabbMax = aabbMax + btVector3(1.0f, 1.0f, 1.0f);

        // TODO: early exit if it's far away
        float awarenessDistance = 2000.f;

        // TODO: get eye position
        // TODO: visualize
        btVector3 rayFrom = BulletGLM::getBulletVec3(character->m_position + glm::vec3(0, 3.3f, 0));
        btVector3 rayTo = rayFrom + BulletGLM::getBulletVec3(character->m_followCamera->front * awarenessDistance);

        btScalar param;
        btVector3 normal;

        intersect = btRayAabb(rayFrom, rayTo, aabbMin, aabbMax, param, normal);

        if (intersect)
        {
            // float lastFrame = (float)glfwGetTime();
            // std::cout << "NPCharacter::avoidAim: intersect with: " << character << ", at: " << lastFrame << std::endl;

            btVector3 aabbCenter = (aabbMin + aabbMax) * 0.5f;
            btVector3 centerToRayDirection = aabbCenter - rayFrom;
            btVector3 centerToRight = BulletGLM::getBulletVec3(character->m_followCamera->right);

            btScalar dotProduct = centerToRayDirection.dot(centerToRight);

            m_controller->m_actionState.left = dotProduct <= 0.0f;
            m_controller->m_actionState.right = dotProduct > 0.0f;
            // m_controller->m_actionState.run = true;

            continue;
        }
    }

    if (!intersect)
    {
        m_controller->m_actionState.left = false;
        m_controller->m_actionState.right = false;
        // m_controller->m_actionState.run = false;
    }
}
