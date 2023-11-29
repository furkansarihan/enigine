#include "exit_car.h"

ExitCar::ExitCar(Character *character, CarController *car)
    : m_character(character),
      m_car(car)
{
    m_doorOpenTime = 25.f;
    m_doorCloseTime = 3000.f;
    m_minInterruptTime = 1500.f;

    float minJumpSpeed = 5.f;
    if (m_car->m_vehicle->m_speed > minJumpSpeed)
        m_jumping = true;
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

    if (m_jumping)
        return;

    Anim *exitPose = getExistPose();
    float animTime = exitPose->m_timer;
    if (animTime < m_minInterruptTime)
    {
        // std::cout << "interrupt: can not interrupt" << std::endl;
        return;
    }

    // std::cout << "interrupt: started" << std::endl;

    m_interruptRequested = true;
    m_character->m_passengerInfo.state = PassengerState::exitInterrupt;

    //
    int index = exitPose->m_animation->m_boneInfoMap["mixamorig:Hips"].id;
    glm::mat4 model = m_character->m_animator->m_globalMatrices[index];

    glm::mat4 model2 = m_character->m_modelMatrix;
    model2 = model2 * model;

    glm::mat3 rotation = glm::mat3(model2);

    m_refPos = CommonUtil::positionFromModel(model2);
    // TODO: correct
    m_refPos.y -= m_character->m_controller->m_halfHeight + 0.3f;
    m_lookDir = rotation * glm::vec3(0.f, 0.f, 1.f);

    m_character->m_controller->m_refFront = m_lookDir;

    exitPose->m_playbackSpeed = 0.f;
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
            float startTime = 400.f;
            getExistPose()->m_timer = startTime;
            m_doorOpened = true;
            m_startAnimBlendReady = false;
        }
        else
        {
            Anim &exitPose = *getExistPose();
            exitPose.m_blendFactor = 1.f;
            exitPose.m_timer = 0.f;
            m_startAnimBlendReady = true;

            Anim &enterPose = *m_character->m_animPoseEnterCar;
            enterPose.m_blendFactor = 0.f;
        }

        m_car->m_controlVehicle = false;

        m_firstUpdate = true;
    }

    updateRefValues();

    m_character->m_position = m_refPos;
    m_character->m_controller->m_refFront = m_lookDir;
    m_character->m_controller->m_refRight = glm::cross(m_lookDir, glm::vec3(0.f, 1.f, 0.f));

    // environment interrupts
    // TODO: check is car moving

    Anim &animPose = *getExistPose();
    Anim &enterAnimPose = *m_character->m_animPoseEnterCar;

    if (animPose.m_blendFactor == 1.f)
        m_startAnimBlendReady = true;

    if (!m_startAnimBlendReady)
    {
        animPose.m_blendFactor += m_stateChangeSpeed;
        animPose.m_blendFactor = std::max(0.0f, std::min(animPose.m_blendFactor, 1.0f));
        return false;
    }

    float animDuration = animPose.m_animation->m_duration;
    float animTime = animPose.m_timer;

    if (m_animFinished && animPose.m_blendFactor != 0.f)
    {
        // wait at the end
        animPose.m_timerActive = false;
        animPose.m_timer = animDuration - 34.f;

        animPose.m_blendFactor -= m_stateChangeSpeed;
        animPose.m_blendFactor = std::max(0.0f, std::min(animPose.m_blendFactor, 1.0f));

        enterAnimPose.m_blendFactor = 0.f;
    }

    if (animPose.m_blendFactor == 0.f)
        m_animBlendReady = true;

    // TODO: better end detection?
    if (m_animFinished)
        return false;

    if (animTime >= animDuration - 34.f)
    {
        if (m_jumping)
        {
            int index = m_character->m_animator->m_animations[0]->m_boneInfoMap["mixamorig:Hips"].id;
            glm::mat4 model = m_character->m_animator->m_finalBoneMatrices[index];
            model = m_character->m_modelMatrix * model;

            btVector3 offsetPos = BulletGLM::getBulletVec3(CommonUtil::positionFromModel(model));
            btQuaternion offsetRot = BulletGLM::getBulletQuat(glm::quat_cast(model));

            m_character->activateRagdoll();
            // TODO: set linear velocity?
            m_character->applyImpulseFullRagdoll(-m_character->m_controller->m_refRight * m_car->m_vehicle->m_speed * 50.f);
            // TODO: sync anim to ragdoll
            m_character->m_ragdoll->resetTransforms(offsetPos, offsetRot);

            Anim &ragdolPose = *m_character->m_animPoseRagdoll;
            ragdolPose.m_blendFactor = 1.f;

            animPose.m_blendFactor = 0.f;
            m_animBlendReady = true;
        }

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
    Anim &exitPose = *getExistPose();
    if (exitPose.m_blendFactor == 0.f)
    {
        // std::cout << "interruptUpdate: end" << std::endl;
        endTask();
        m_interrupted = true;
        return;
    }

    exitPose.m_blendFactor -= m_stateChangeSpeed * 2.f;
    exitPose.m_blendFactor = std::max(0.0f, std::min(exitPose.m_blendFactor, 1.0f));

    m_character->m_position = glm::mix(m_character->m_position, m_refPos, m_posFactor * 2.f);
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

    m_character->m_animPoseEnterCar->m_timerActive = true;
    Anim *exitAnim = getExistPose();
    exitAnim->m_timerActive = true;
    exitAnim->m_playbackSpeed = 1.f;
}

void ExitCar::updateRefValues()
{
    glm::vec3 corner0 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(1.f, 0.f, 0.f)));
    glm::vec3 corner1 = CommonUtil::positionFromModel(m_car->translateOffset(glm::vec3(-1.f, 0.f, 0.f)));
    m_lookDir = glm::normalize(corner1 - corner0);

    m_refPos = CommonUtil::positionFromModel(m_car->translateOffset(m_car->m_animDoorOffset));
}

Anim *ExitCar::getExistPose()
{
    return m_jumping ? m_character->m_animPoseJumpCar : m_character->m_animPoseExitCar;
}
