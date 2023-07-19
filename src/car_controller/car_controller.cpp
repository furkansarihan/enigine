#include "car_controller.h"

CarController::CarController(PhysicsWorld *physicsWorld, ResourceManager *resourceManager, Camera *followCamera, glm::vec3 position)
    : m_followCamera(followCamera)
{
    m_vehicle = new Vehicle(physicsWorld, resourceManager, position);
    m_vehicle->m_carChassis->setUserPointer(this);

    m_exhausParticle = new ParticleEngine(followCamera);
    m_exhausParticle->m_particlesPerSecond = 100.f;
    m_exhausParticle->m_randomness = 0.5f;
    m_exhausParticle->m_minVelocity = 0.1f;
    m_exhausParticle->m_maxVelocity = 0.35f;
    m_exhausParticle->m_minDuration = 1.0f;
    m_exhausParticle->m_maxDuration = 2.0f;
    m_exhausParticle->m_particleScale = 0.1f;

    // models
    m_models.carBody = resourceManager->getModel("../src/assets/car/body.gltf", false);
    m_models.carHood = resourceManager->getModel("../src/assets/car/hood.gltf", false);
    m_models.carTrunk = resourceManager->getModel("../src/assets/car/trunk.gltf", false);
    m_models.wheelModels[0] = m_models.carWheelFL = resourceManager->getModel("../src/assets/car/wheel-fl.gltf", false);
    m_models.wheelModels[1] = m_models.carWheelFR = resourceManager->getModel("../src/assets/car/wheel-fr.gltf", false);
    m_models.wheelModels[2] = m_models.carWheelRL = resourceManager->getModel("../src/assets/car/wheel-rl.gltf", false);
    m_models.wheelModels[3] = m_models.carWheelRR = resourceManager->getModel("../src/assets/car/wheel-rr.gltf", false);
    m_models.doorModels[0] = m_models.carDoorFL = resourceManager->getModel("../src/assets/car/door-fl.gltf", false);
    m_models.doorModels[1] = m_models.carDoorFR = resourceManager->getModel("../src/assets/car/door-fr.gltf", false);
    m_models.doorModels[2] = m_models.carDoorRL = resourceManager->getModel("../src/assets/car/door-rl.gltf", false);
    m_models.doorModels[3] = m_models.carDoorRR = resourceManager->getModel("../src/assets/car/door-rr.gltf", false);
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
    for (int i = 0; i < 4; i++)
        m_doorModels[i] = getDoorModel(i);
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

glm::mat4 CarController::translateOffset(glm::vec3 offset)
{
    glm::mat4 model = m_vehicle->m_chassisModel;
    model = glm::translate(model, offset);
    model = model * m_carModel;
    return model;
}

glm::mat4 CarController::getDoorModel(int i)
{
    glm::mat4 model;
    if (m_vehicle->m_doors[i].hingeState == HingeState::deactive)
    {
        model = m_vehicle->m_chassisModel;
        model = glm::translate(model, BulletGLM::getGLMVec3(m_vehicle->m_doors[i].posOffset));
        model = model * m_carModel;
    }
    else
    {
        // TODO: remove this workaround
        btTransform transform;
        if (m_vehicle->m_speed > 10.f)
            m_vehicle->m_doors[i].body->getMotionState()->getWorldTransform(transform);
        else
            transform = m_vehicle->m_doors[i].body->getWorldTransform();

        transform.getOpenGLMatrix((btScalar *)&model);
        model = glm::rotate(model, m_doorRotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, m_doorRotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, m_doorRotation.z, glm::vec3(0, 0, 1));
        model = glm::rotate(model, m_bodyRotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, m_bodyRotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, m_bodyRotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(m_scale));
    }

    return model;
}

// TODO: renderer
void CarController::render(Shader shader, glm::mat4 viewProjection, const std::string &uniformName, bool drawOpaque)
{
    glm::mat4 model = m_vehicle->m_chassisModel;
    model = glm::translate(model, m_bodyOffset);
    model = model * m_carModel;
    shader.setMat4(uniformName, viewProjection * model);
    m_models.carBody->draw(shader, drawOpaque);

    for (int i = 0; i < 4; i++)
    {
        glm::mat4 model;
        m_vehicle->m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix((btScalar *)&model);
        model = glm::translate(model, m_wheelOffset);
        model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, m_rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(m_wheelScale));
        shader.setMat4(uniformName, viewProjection * model);
        m_models.wheelModels[i]->draw(shader, drawOpaque);
    }

    // hood
    model = m_vehicle->m_chassisModel;
    model = glm::translate(model, m_hoodOffset);
    model = model * m_carModel;
    shader.setMat4(uniformName, viewProjection * model);
    m_models.carHood->draw(shader, drawOpaque);

    // trunk
    model = m_vehicle->m_chassisModel;
    model = glm::translate(model, m_trunkOffset);
    model = model * m_carModel;
    shader.setMat4(uniformName, viewProjection * model);
    m_models.carTrunk->draw(shader, drawOpaque);

    // doors
    for (int i = 0; i < 4; i++)
    {
        shader.setMat4(uniformName, viewProjection * m_doorModels[i]);
        m_models.doorModels[i]->draw(shader, drawOpaque);
    }
}
