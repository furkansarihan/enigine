#include "playable_character.h"

PCharacter::PCharacter(TaskManager *taskManager, SoundEngine *soundEngine, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : Character(taskManager, resourceManager, physicsWorld, followCamera),
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

    m_smokeParticle = new ParticleEngine(followCamera);
    m_smokeParticle->m_particlesPerSecond = 100.f;
    m_smokeParticle->m_randomness = 0.5f;
    m_smokeParticle->m_minVelocity = 0.1f;
    m_smokeParticle->m_maxVelocity = 5.0f;
    m_smokeParticle->m_minDuration = 1.0f;
    m_smokeParticle->m_maxDuration = 3.0f;
    m_smokeParticle->m_particleScale = 4.0f;

    m_muzzleFlash = new ParticleEngine(followCamera);
    m_muzzleFlash->m_particlesPerSecond = 250.f;
    m_muzzleFlash->m_randomness = 1.0f;
    m_muzzleFlash->m_minVelocity = 0.1f;
    m_muzzleFlash->m_maxVelocity = 0.4f;
    m_muzzleFlash->m_minDuration = 0.2f;
    m_muzzleFlash->m_maxDuration = 0.6f;
    m_muzzleFlash->m_particleScale = 0.15f;

    m_controlCharacter = true;
    m_followCharacter = true;
}

PCharacter::~PCharacter()
{
    for (int i = 0; i < 6; i++)
        m_soundEngine->deleteSource(m_fireSounds[i]);
    m_soundEngine->deleteSound(m_fireSoundBuffer);

    delete m_smokeParticle;
    delete m_muzzleFlash;
}

void PCharacter::update(GLFWwindow *window, float deltaTime)
{
    Character::update(deltaTime);

    // particle
    m_smokeParticle->update(deltaTime);
    m_smokeParticle->m_position = m_lastFireHit;

    m_muzzleFlash->m_particlesPerSecond = m_firing ? 250.f : 0.f;
    m_muzzleFlash->update(deltaTime);
    m_muzzleFlash->m_position = m_muzzlePosition;
    m_muzzleFlash->m_direction = m_muzzleDirection;

    updatePistolModelMatrix();

    // input
    {

        m_controller->recieveInput(window, deltaTime);
        bool anyInput = m_controller->m_actionState.forward ||
                        m_controller->m_actionState.backward ||
                        m_controller->m_actionState.left ||
                        m_controller->m_actionState.right ||
                        m_controller->m_actionState.jump;
        // TODO: better way?
        if (anyInput)
        {
            if (m_passengerInfo.state == PassengerState::entering)
                cancelEnterCar();
            else if (m_passengerInfo.state == PassengerState::exiting)
                interruptExitCar();
        }

        if (!m_controlCharacter || m_passengerInfo.state != PassengerState::outside)
        {
            m_controller->m_actionState.forward = false;
            m_controller->m_actionState.backward = false;
            m_controller->m_actionState.left = false;
            m_controller->m_actionState.right = false;
            m_controller->m_actionState.jump = false;
        }
        else if (m_controlCharacter)
            m_controller->updateFollowVectors();
    }

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

        if (!m_controlCharacter)
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
        resetRagdoll();
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        float now = (float)glfwGetTime();
        if (m_passengerInfo.state == PassengerState::outside)
        {
            if (now - m_lastCarRequest > m_carRequestLimit)
            {
                m_lastCarRequest = now;
                enterNearestCar();
            }
        }
        else if (m_passengerInfo.state == PassengerState::inside)
        {
            if (now - m_lastCarRequest > m_carRequestLimit)
            {
                m_lastCarRequest = now;
                exitFromCar();
            }
        }
    }

    // TODO: single key event
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        m_controller->m_aimLocked = true;
        cancelEnterCar();
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        m_controller->m_aimLocked = false;
}

void PCharacter::updatePistolModelMatrix()
{
    int index = m_animator->m_animations[0]->m_BoneInfoMap["mixamorig:RightHand"].id;
    glm::mat4 model = m_animator->m_globalMatrices[index];

    glm::mat4 model2 = m_modelMatrix;

    model2 = model2 * model;
    glm::mat4 model3 = model2;

    model2 = glm::translate(model2, m_pistolOffset);
    model2 = model2 * glm::mat4_cast(m_pistolOrientation);
    model2 = glm::scale(model2, glm::vec3(m_pistolScale));

    m_pistolModel = model2;

    // muzzle
    model3 = glm::translate(model3, m_pistolOffset + m_muzzleOffset);
    model3 = model3 * glm::mat4_cast(m_pistolOrientation);
    model3 = glm::scale(model3, glm::vec3(m_pistolScale));

    m_muzzlePosition = CommonUtil::positionFromModel(model3);
    m_muzzleDirection = glm::normalize(glm::mat3(m_pistolModel) * glm::vec3(0.f, 0.f, 1.f));
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
        // TODO: modifying in a thread?
        m_lastFireHit = BulletGLM::getGLMVec3(callback.m_hitPointWorld);

        // std::cout << "PCharacter::fireWeapon: hit" << std::endl;
        const btRigidBody *rb = btRigidBody::upcast(callback.m_collisionObject);
        // TODO: O(1) lookup
        for (int i = 0; i < m_npcList.size(); i++)
        {
            if (m_npcList[i]->m_rigidbody == rb)
                m_npcList[i]->activateRagdoll(m_followCamera->front * m_impulseStrength);
        }
    }
    // else
    //     std::cout << "PCharacter::fireWeapon: no hit" << std::endl;
}

void PCharacter::playFireSound()
{
    m_lastFireSound = (m_lastFireSound + 1) % m_concurrentSoundCount;
    SoundSource &fireSound = m_fireSounds[m_lastFireSound];

    int index = rand() % m_concurrentSoundCount + 0;

    m_soundEngine->setSourcePosition(fireSound, m_position.x, m_position.y, m_position.z);
    m_soundEngine->setPlayerPosition(fireSound, (float)index * 2.f);
    m_soundEngine->playSource(fireSound);

    auto soundCallback = [&]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        m_soundEngine->stopSource(fireSound);
    };
    std::thread workerThread(soundCallback);
    workerThread.detach();
}
