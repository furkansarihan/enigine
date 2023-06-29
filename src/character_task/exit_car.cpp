#include "exit_car.h"

ExitCar::ExitCar(Character *character, CarController *car)
    : m_character(character),
      m_car(car)
{
}

ExitCar::~ExitCar()
{
}

void ExitCar::interrupt()
{
    m_interrupted = true;
    // TODO:
}

// TODO: with animation
bool ExitCar::update()
{
    if (m_interrupted)
        return true;

    m_car->m_controlVehicle = false;
    m_car->m_followVehicle = false;
    m_character->m_controlCharacter = true;
    m_character->m_followCharacter = true;
    m_character->m_controller->m_rotate = false;
    m_character->m_syncPositionFromPhysics = true;
    m_character->passengerInfo.state = PassengerState::outside;
    m_character->passengerInfo.car = nullptr;

    m_character->m_animator->m_animations[18]->m_timed = false;
    AnimPose &animPose = m_character->getEnterCarAnim();
    animPose.blendFactor = 0.f;

    updateRefValues();

    btVector3 origin = m_character->m_rigidbody->getWorldTransform().getOrigin();
    m_refPos.y = origin.getY();
    glm::vec3 position = glm::mix(BulletGLM::getGLMVec3(origin), m_refPos, m_posFactor);
    m_character->m_rigidbody->getWorldTransform().setOrigin(BulletGLM::getBulletVec3(position));
    m_character->activateCollider();

    return true;
}

void ExitCar::updateRefValues()
{
    glm::vec3 doorOffset = m_car->m_animDoorOffset;

    glm::vec3 corner0 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(1.f, 0.f, 0.f)));
    glm::vec3 corner1 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(-1.f, 0.f, 0.f)));
    glm::vec3 doorDir = glm::normalize(corner1 - corner0);

    m_refPos = CommonUtil::positionFromModel(m_car->translateOffset(doorOffset));
    // m_refPos.y = m_character->m_position.y;
    m_lookDir = glm::normalize(glm::vec3(doorDir.x, doorDir.y, doorDir.z));
}