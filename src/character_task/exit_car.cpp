#include "exit_car.h"

ExitCar::ExitCar(Character *character, CarController *car)
    : m_character(character),
      m_car(car)
{
    m_doorOpenTime = 100.f;
    m_doorCloseTime = 3000.f;
    m_minInterruptTime = 2000.f;
}

ExitCar::~ExitCar()
{
}

void ExitCar::interrupt()
{
    if (m_interruptRequested)
    {
        // std::cout << "interrupt: already requested" << std::endl;
        return;
    }

    float animTime = m_character->m_animator->m_timers[19];
    if (animTime < m_minInterruptTime)
    {
        // std::cout << "interrupt: can not interrupt" << std::endl;
        return;
    }

    // std::cout << "interrupt: started" << std::endl;

    m_interruptRequested = true;

    //
    int index = m_character->m_animator->m_animations[0]->m_BoneInfoMap["mixamorig:Hips"].id;
    glm::mat4 model = m_character->m_animator->m_globalMatrices[index];

    glm::mat4 model2(1.0f);
    model2 = glm::translate(model2, m_character->m_position);
    model2 = glm::rotate(model2, m_character->m_rotation.y * (1.0f - m_character->getRagdolPose().blendFactor), glm::vec3(0, 1, 0));
    model2 = glm::scale(model2, glm::vec3(m_character->m_scale));

    model2 = model2 * model;

    glm::mat3 rotation = glm::mat3(model2);

    m_refPos = CommonUtil::positionFromModel(model2);
    m_lookDir = rotation * glm::vec3(0.f, 0.f, 1.f);

    m_character->m_controller->m_refFront = m_lookDir;

    m_character->m_animator->m_animations[19]->m_playbackSpeed = 0.f;
}

// TODO: with animation
bool ExitCar::update()
{
    if (m_interrupted)
        return true;

    if (m_interruptRequested)
    {
        interruptUpdate();
        return false;
    }

    if (m_animBlendReady)
    {
        endTask();
        return true;
    }

    if (!m_firstUpdate)
    {
        // std::cout << "EnterCar::update: m_firstUpdate" << std::endl;
        if (m_car->m_vehicle->isDoorOpen(Door::frontLeft))
        {
            float startTime = 600.f;
            m_character->m_animator->setAnimTime(19, startTime);
            m_doorOpened = true;
            m_startAnimBlendReady = false;
        }
        else
        {
            m_character->m_animator->setAnimTime(19, 0.f);
            m_startAnimBlendReady = true;

            AnimPose &enterPose = m_character->getEnterCarAnim();
            enterPose.blendFactor = 0.f;

            AnimPose &animPose = m_character->getExitCarAnim();
            animPose.blendFactor = 1.f;
        }

        m_car->m_controlVehicle = false;

        updateRefValues();

        m_character->m_position = m_refPos;
        m_character->m_controller->m_refFront = m_lookDir;
        m_character->m_controller->m_refRight = glm::cross(m_lookDir, glm::vec3(0.f, 1.f, 0.f));

        m_firstUpdate = true;
    }

    AnimPose &animPose = m_character->getExitCarAnim();
    AnimPose &enterAnimPose = m_character->getEnterCarAnim();

    if (animPose.blendFactor == 1.f)
        m_startAnimBlendReady = true;

    if (!m_startAnimBlendReady)
    {
        animPose.blendFactor += m_stateChangeSpeed;
        animPose.blendFactor = std::max(0.0f, std::min(animPose.blendFactor, 1.0f));

        enterAnimPose.blendFactor = 1.f - animPose.blendFactor;
        return false;
    }

    float animDuration = m_character->m_animator->m_animations[19]->m_Duration;

    if (m_animFinished && animPose.blendFactor != 0.f)
    {
        // wait at the end
        m_character->m_animator->m_animations[19]->m_timed = true;
        animPose.time = animDuration - 34.f;

        animPose.blendFactor -= m_stateChangeSpeed;
        animPose.blendFactor = std::max(0.0f, std::min(animPose.blendFactor, 1.0f));
    }

    if (animPose.blendFactor == 0.f)
        m_animBlendReady = true;

    // TODO: better end detection?
    if (m_animFinished)
        return false;

    float animTime = m_character->m_animator->m_timers[19];
    if (animTime >= animDuration - 34.f)
    {
        m_animFinished = true;
        return false;
    }

    if (!m_doorOpened && animTime > m_doorOpenTime)
    {
        m_car->m_vehicle->openDoor(Door::frontLeft);
        m_doorOpened = true;
    }
    else if (!m_doorClosed && animTime > m_doorCloseTime)
    {
        m_car->m_vehicle->closeDoor(Door::frontLeft);
        m_doorClosed = true;
    }

    return false;
}

void ExitCar::interruptUpdate()
{
    AnimPose &animPose = m_character->getExitCarAnim();

    if (animPose.blendFactor == 0.f)
    {
        // std::cout << "interruptUpdate: end" << std::endl;
        endTask();
        m_interrupted = true;
        return;
    }

    // wait at the end
    animPose.blendFactor -= m_stateChangeSpeed;
    animPose.blendFactor = std::max(0.0f, std::min(animPose.blendFactor, 1.0f));

    m_character->m_position = glm::mix(m_character->m_position, m_refPos, m_posFactor);
}

void ExitCar::endTask()
{
    m_car->m_followVehicle = false;

    btVector3 origin = m_character->m_rigidbody->getWorldTransform().getOrigin();
    m_refPos.y += m_character->m_controller->m_halfHeight;
    m_character->m_rigidbody->getWorldTransform().setOrigin(BulletGLM::getBulletVec3(m_refPos));
    m_character->m_rigidbody->setLinearVelocity(btVector3(0, 0, 0));
    m_character->activateCollider();

    m_character->m_controlCharacter = true;
    m_character->m_followCharacter = true;
    m_character->m_controller->m_rotate = false;
    m_character->m_syncPositionFromPhysics = true;
    m_character->m_passengerInfo.state = PassengerState::outside;
    m_character->m_passengerInfo.car = nullptr;

    m_character->m_animator->m_animations[18]->m_timed = false;
    m_character->m_animator->m_animations[19]->m_timed = false;
    m_character->m_animator->m_animations[19]->m_playbackSpeed = 1.f;
}

void ExitCar::updateRefValues()
{
    glm::vec3 doorOffset = m_car->m_animDoorOffset;

    glm::vec3 corner0 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(1.f, 0.f, 0.f)));
    glm::vec3 corner1 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(-1.f, 0.f, 0.f)));
    glm::vec3 doorDir = glm::normalize(corner1 - corner0);

    m_refPos = CommonUtil::positionFromModel(m_car->translateOffset(doorOffset));
    m_lookDir = glm::normalize(glm::vec3(doorDir.x, doorDir.y, doorDir.z));
}