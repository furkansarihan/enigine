#include "playable_character.h"

PCharacter::PCharacter(SoundEngine *soundEngine, ShaderManager *m_shaderManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : Character(m_shaderManager, physicsWorld, followCamera),
      m_soundEngine(soundEngine)
{
    try
    {
        m_fireSoundBuffer = m_soundEngine->loadSound("../src/assets/sounds/colt-fire.mp3");
    }
    catch (const char *e)
    {
        std::cerr << e << std::endl;
        return;
    }

    for (int i = 0; i < m_concurrentSoundCount; i++)
        m_fireSounds.push_back(m_soundEngine->createSource(m_fireSoundBuffer));

    for (int i = 0; i < m_concurrentSoundCount; i++)
        m_soundEngine->setSourceLooping(m_fireSounds[i], AL_FALSE);
}

PCharacter::~PCharacter()
{
    for (int i = 0; i < 6; i++)
        m_soundEngine->deleteSource(m_fireSounds[i]);
    m_soundEngine->deleteSound(m_fireSoundBuffer);
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
        // TODO: change state
        if (!m_controller->m_aimLocked)
            return;

        float now = (float)glfwGetTime();
        if (now - m_lastFire > m_fireLimit)
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
    m_firing = true;
    m_animator->setAnimTime(17, m_fireAnimStartTime);
    auto fireCallback = [&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_fireAnimTimeMilli / 2));
        shootRay();
        playFireSound();
        std::this_thread::sleep_for(std::chrono::milliseconds(m_fireAnimTimeMilli / 2));
        m_firing = false;
    };
    std::thread workerThread(fireCallback);
    workerThread.detach();
}

void PCharacter::shootRay()
{
    btVector3 from = BulletGLM::getBulletVec3(m_followCamera->position);
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

void PCharacter::playFireSound()
{
    m_lastFireSound = (m_lastFireSound + 1) % m_concurrentSoundCount;
    SoundSource &fireSound = m_fireSounds[m_lastFireSound];

    std::random_device rd;
    std::mt19937 rng(rd());
    int minVal = 0;
    int maxVal = m_concurrentSoundCount - 1;
    std::uniform_int_distribution<int> distribution(minVal, maxVal);
    int index;

    m_soundEngine->setPlaybackPosition(fireSound, index * 2);
    m_soundEngine->playSource(fireSound);

    auto soundCallback = [&]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        m_soundEngine->stopSource(fireSound);
    };
    std::thread workerThread(soundCallback);
    workerThread.detach();
}
