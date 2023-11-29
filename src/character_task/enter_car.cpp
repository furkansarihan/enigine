#include "enter_car.h"

EnterCar::EnterCar(Character *character, CarController *car)
    : m_character(character),
      m_car(car)
{
    m_doorOpenTime = 700.f;
    m_doorCloseTime = 3600.f;
}

EnterCar::~EnterCar()
{
}

void EnterCar::interrupt()
{
    m_interrupted = true;

    m_car->m_controlVehicle = false;
    m_car->m_followVehicle = false;
    m_character->activateCollider();
    m_character->m_controlCharacter = true;
    m_character->m_followCharacter = true;
    m_character->m_controller->m_rotate = false;
    m_character->m_syncPositionFromPhysics = true;
    m_character->m_passengerInfo.state = PassengerState::outside;
    m_character->m_passengerInfo.car = nullptr;

    // TODO: animation adaptation
    m_character->m_animPoseEnterCar->m_blendFactor = 0.f;
}

// TODO: animation adaptation for car distance
// TODO: interrupt - check distance
bool EnterCar::update()
{
    if (m_interrupted)
        return true;

    // TODO: better way?
    if (m_character->m_passengerInfo.exitRequested)
    {
        // std::cout << "EnterCar::update: exitRequested" << std::endl;
        m_character->m_passengerInfo.exitRequested = false;
        return true;
    }

    if (!m_firstUpdate)
    {
        // std::cout << "EnterCar::update: m_firstUpdate" << std::endl;
        if (m_car->m_vehicle->isDoorOpen(Door::frontLeft))
        {
            float startTime = 680.f;
            m_character->m_animPoseEnterCar->m_timer = startTime;
            m_doorOpened = true;
        }
        else
        {
            m_character->m_animPoseEnterCar->m_timer = 0.f;
        }
        m_character->m_syncPositionFromPhysics = false;
        m_character->m_controller->m_rotate = true;
        m_character->inactivateCollider();
        m_firstUpdate = true;
    }

    Anim &animPose = *m_character->m_animPoseEnterCar;
    animPose.m_blendFactor += m_stateChangeSpeed;
    animPose.m_blendFactor = std::max(0.0f, std::min(animPose.m_blendFactor, 1.0f));

    // TODO: check if anim played - stop at the end
    float animDuration = animPose.m_animation->m_duration;
    if (!m_sitting)
    {
        // TODO: better end detection?
        // float animTime = m_character->m_animator->m_timers[18];
        float animTime = 0.f;
        m_sitting = animTime >= animDuration - 34.f;
        if (m_sitting)
        {
            m_car->m_controlVehicle = true;
            m_car->m_followVehicle = true;
            m_character->m_controlCharacter = false;
            m_character->m_followCharacter = false;
            m_character->m_passengerInfo.state = PassengerState::inside;
            m_character->m_passengerInfo.car = m_car;
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
    }
    else
    {
        animPose.m_timerActive = false;
        animPose.m_timer = animDuration - 34.f;
    }

    updateRefValues();

    if (!m_positionReached)
    {
        // TODO: curve
        m_character->m_position = glm::mix(m_character->m_position, m_refPos, m_posFactor);
        float distance = std::abs(glm::distance(m_character->m_position, m_refPos));
        m_positionReached = distance < 0.03f;
    }

    if (!m_rotationReached)
    {
        m_character->m_controller->m_refFront = m_lookDir;
        m_character->m_controller->m_refRight = glm::cross(m_lookDir, glm::vec3(0.f, 1.f, 0.f));

        float distance = std::abs(glm::distance(m_character->m_controller->m_lookDir, m_lookDir));

        m_rotationReached = distance < 0.01f;
    }

    return false;
}

void EnterCar::updateRefValues()
{
    glm::vec3 corner0 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(1.f, 0.f, 0.f)));
    glm::vec3 corner1 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(-1.f, 0.f, 0.f)));
    m_lookDir = glm::normalize(corner1 - corner0);

    m_refPos = CommonUtil::positionFromModel(m_car->translateOffset(m_car->m_animDoorOffset));
}
