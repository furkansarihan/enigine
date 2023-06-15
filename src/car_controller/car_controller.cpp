#include "car_controller.h"

CarController::CarController(PhysicsWorld *physicsWorld, Camera *followCamera, glm::vec3 position)
    : m_followCamera(followCamera)
{
    m_vehicle = new Vehicle(physicsWorld, position);

    m_exhausParticle = new ParticleEngine(followCamera);
    m_exhausParticle->m_particlesPerSecond = 100.f;
    m_exhausParticle->m_randomness = 0.5f;
    m_exhausParticle->m_minVelocity = 0.1f;
    m_exhausParticle->m_maxVelocity = 0.35f;
    m_exhausParticle->m_minDuration = 1.0f;
    m_exhausParticle->m_maxDuration = 2.0f;
    m_exhausParticle->m_particleScale = 0.1f;

    m_doorOffsets[0] = glm::vec3(1.51f, 1.95f, 0.62);
    m_doorOffsets[1] = glm::vec3(-1.46f, 1.95f, 0.62);
    m_doorOffsets[2] = glm::vec3(1.46f, 2.01f, -1.33);
    m_doorOffsets[3] = glm::vec3(-1.46f, 2.01f, -1.32);
}

CarController::~CarController()
{
    delete m_vehicle;
    delete m_exhausParticle;
}

void CarController::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(BulletGLM::getBulletVec3(m_vehicle->m_position));
        m_vehicle->resetVehicle(tr);
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(m_vehicle->m_carChassis->getWorldTransform().getOrigin() + btVector3(0, 5, 0));
        m_vehicle->resetVehicle(tr);
    }
}

void CarController::staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    CarController *cc = (CarController *)glfwGetWindowUserPointer(window);
    cc->keyCallback(window, key, scancode, action, mods);
}

void CarController::update(GLFWwindow *window, float deltaTime)
{
    m_vehicle->update(deltaTime);
    if (m_controlVehicle)
        m_vehicle->recieveInput(window);

    updateModels();
    updateExhaust(window, deltaTime);
    followCar();
}

void CarController::followCar()
{
    if (!m_followVehicle)
        return;

    float speed = m_vehicle->m_speed;
    float oneOverSpeed = 1.f / (speed + 1.f);
    // udpate follow specs
    Follow &follow = m_follow;
    follow.gapTarget = speed * follow.gapFactor + std::fabs(m_vehicle->gVehicleSteering) * follow.steeringFactor * oneOverSpeed;
    follow.gap = CommonUtil::lerp(follow.gap, follow.gapTarget, follow.gapSpeed);
    m_pos = CommonUtil::positionFromModel(m_vehicle->m_chassisModel);
    m_followCamera->position = m_pos - m_followCamera->front * glm::vec3(follow.distance + follow.gap) + follow.offset;

    if (!m_followCamera->moving)
    {
        glm::vec3 frontTarget = BulletGLM::getGLMVec3(m_vehicle->m_vehicle->getForwardVector());
        // TODO: lower vertical when too fast
        frontTarget.y -= follow.angleFactor * (speed / follow.angularSpeedRange);
        frontTarget = glm::normalize(frontTarget);
        // TODO: slow angular follow while sudden change
        if (m_vehicle->m_vehicle->getCurrentSpeedKmHour() < 0.f)
        {
            frontTarget.x *= -1.f;
            frontTarget.z *= -1.f;
        }
        float dot = glm::abs(glm::dot(m_followCamera->front, frontTarget));
        if (dot < 0.999f)
        {
            float frontFactor = std::max(0.f, std::min(follow.angleSpeed * follow.angleVelocity * follow.move, 1.0f));
            m_followCamera->front = glm::normalize(glm::slerp(m_followCamera->front, frontTarget, frontFactor));
            m_followCamera->up = m_followCamera->worldUp;
            m_followCamera->right = glm::cross(m_followCamera->front, m_followCamera->up);
        }
    }

    follow.angleVelocityTarget = m_followCamera->moving ? 0.f : 1.f;
    follow.angleVelocity = CommonUtil::lerp(follow.angleVelocity, follow.angleVelocityTarget, follow.angleVelocitySpeed);
    follow.angleVelocity = std::max(0.f, std::min(follow.angleVelocity, 1.f));
    follow.moveTarget = (m_vehicle->m_inAction ? 1.f : 0.f) * (speed / follow.moveSpeedRange);
    follow.move = CommonUtil::lerp(follow.move, follow.moveTarget, follow.moveSpeed);
    follow.move = std::max(0.f, std::min(follow.move, 1.f));
}

void CarController::updateModels()
{
    m_carModel = glm::mat4(1.0f);
    m_carModel = glm::rotate(m_carModel, m_rotation.x, glm::vec3(1, 0, 0));
    m_carModel = glm::rotate(m_carModel, m_rotation.y, glm::vec3(0, 1, 0));
    m_carModel = glm::rotate(m_carModel, m_rotation.z, glm::vec3(0, 0, 1));
    m_carModel = glm::rotate(m_carModel, m_bodyRotation.x, glm::vec3(1, 0, 0));
    m_carModel = glm::rotate(m_carModel, m_bodyRotation.y, glm::vec3(0, 1, 0));
    m_carModel = glm::rotate(m_carModel, m_bodyRotation.z, glm::vec3(0, 0, 1));
    m_carModel = glm::scale(m_carModel, glm::vec3(m_scale));
}

void CarController::updateExhaust(GLFWwindow *window, float deltaTime)
{
    m_exhausParticle->update(deltaTime);
    glm::mat4 exhaustModel = m_vehicle->m_chassisModel;
    exhaustModel = glm::translate(exhaustModel, m_exhaustOffset);
    exhaustModel = glm::rotate(exhaustModel, m_exhaustRotation.x, glm::vec3(1, 0, 0));
    exhaustModel = glm::rotate(exhaustModel, m_exhaustRotation.y, glm::vec3(0, 1, 0));
    exhaustModel = glm::rotate(exhaustModel, m_exhaustRotation.z, glm::vec3(0, 0, 1));
    exhaustModel = exhaustModel * m_carModel;
    m_exhausParticle->m_position = CommonUtil::positionFromModel(exhaustModel);
    m_exhausParticle->m_direction = glm::normalize(glm::mat3(exhaustModel) * glm::vec3(0.f, 0.f, 1.f));

    float maxParticlesPerSecond = 250.0f;
    float maxSpeed = 10.0f;

    float particlesPerSecond = 0.0f;
    if (m_vehicle->m_speed <= maxSpeed)
        particlesPerSecond = (1.f - m_vehicle->m_speed / maxSpeed) * maxParticlesPerSecond;

    m_exhausParticle->m_particlesPerSecond = particlesPerSecond;
}
