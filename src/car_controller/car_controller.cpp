#include "car_controller.h"

CarController::CarController(GLFWwindow *window,
                             ShaderManager *shaderManager,
                             RenderManager *renderManager,
                             ResourceManager *resourceManager,
                             Vehicle *vehicle,
                             Camera *followCamera,
                             Models models,
                             int exhaustCount)
    : m_window(window),
      m_vehicle(vehicle),
      m_followCamera(followCamera),
      m_models(models),
      m_exhaustCount(exhaustCount)
{
    m_vehicle->m_carChassis->setUserPointer(this);

    m_follow.offset = glm::vec3(0.0f, 1.f, 0.0f);
    m_follow.distance = 6.0f;
    m_follow.stretch = 0.0f;
    m_follow.stretchTarget = 0.0f;
    m_follow.stretchMax = 6.f;
    m_follow.stretchSpeed = 0.75f;
    m_follow.stretchSteeringFactor = 4.0f;
    m_follow.yOffset = 0.2f;
    m_follow.yStretchFactor = 0.1f;
    m_follow.angularSpeed = 0.05f;

    for (int i = 0; i < exhaustCount; i++)
    {
        ParticleEngine *particle = new ParticleEngine(resourceManager, renderManager->quad, followCamera);
        particle->m_particlesPerSecond = 100.f;
        particle->m_randomness = 0.5f;
        particle->m_minVelocity = 0.1f;
        particle->m_maxVelocity = 0.35f;
        particle->m_minDuration = 1.0f;
        particle->m_maxDuration = 2.0f;
        particle->m_particleScale = 0.1f;

        m_exhausParticles.push_back(particle);
    }

    for (int i = 0; i < 4; i++)
    {
        bool front = i < 2;
        ParticleEngine *particle = new ParticleEngine(resourceManager,m_models.smokeParticleModel, followCamera);
        particle->m_particlesPerSecond = 0.f;
        particle->m_randomness = 1.f;
        particle->m_minVelocity = front ? 0.2f : 0.4f;
        particle->m_maxVelocity = front ? 0.3f : 1.f;
        particle->m_minDuration = 2.f;
        particle->m_maxDuration = 3.f;
        particle->m_particleScale = front ? 0.5f : 1.2f;

        m_tireSmokeParticles.push_back(particle);
    }

    shaderManager->addShader(ShaderDynamic(&m_exhaustShader, "assets/shaders/smoke.vs", "assets/shaders/exhaust.fs"));
    shaderManager->addShader(ShaderDynamic(&m_tireSmokeShader, "assets/shaders/tire-smoke.vs", "assets/shaders/tire-smoke.fs"));

    eTransform offset;
    m_linkBody = new TransformLinkRigidBody(m_vehicle->m_carChassis, offset);
    m_bodySource = RenderSourceBuilder()
                       .setModel(m_models.carBody)
                       .setTransformLink(m_linkBody)
                       .build();
    renderManager->addSource(m_bodySource);

    if (m_models.carHood)
    {
        m_linkHood = new TransformLinkRigidBody(m_vehicle->m_carChassis, offset);
        m_bodyHood = RenderSourceBuilder()
                         .setModel(m_models.carHood)
                         .setTransformLink(m_linkHood)
                         .build();
        renderManager->addSource(m_bodyHood);
    }

    if (m_models.carTrunk)
    {
        m_linkTrunk = new TransformLinkRigidBody(m_vehicle->m_carChassis, offset);
        m_bodyTrunk = RenderSourceBuilder()
                          .setModel(m_models.carTrunk)
                          .setTransformLink(m_linkTrunk)
                          .build();
        renderManager->addSource(m_bodyTrunk);
    }

    for (int i = 0; i < 4; i++)
    {
        eTransform offset;
        // TODO: better - rotate right wheels
        if (i == 1 || i == 3)
            offset.setRotation(glm::quat(0.f, 0.f, 0.f, -1.f));

        m_linkWheels.push_back(new TransformLinkWheel(m_vehicle->m_vehicle, i, offset));
        m_wheelSources[i] = RenderSourceBuilder()
                                .setModel(m_models.wheelBase)
                                .setTransformLink(m_linkWheels[i])
                                .build();
        renderManager->addSource(m_wheelSources[i]);
    }

    int doorCount = m_vehicle->m_type == VehicleType::coupe ? 2 : 4;
    for (int i = 0; i < doorCount; i++)
    {
        eTransform offsetD;
        offsetD.setPosition(BulletGLM::getGLMVec3(m_vehicle->m_doors[i].posOffset));
        m_linkDoors.push_back(new TransformLinkDoor(m_vehicle, i, offset, offsetD));
        m_doorSources[i] = RenderSourceBuilder()
                               .setModel(m_models.doorFront)
                               .setFaceCullType(i % 2 == 0 ? FaceCullType::backFaces : FaceCullType::frontFaces)
                               .setTransformLink(m_linkDoors[i])
                               .build();
        renderManager->addSource(m_doorSources[i]);
    }

    // exhaust
    for (size_t i = 0; i < exhaustCount; i++)
    {
        m_linkExhausts.push_back(new TransformLinkRigidBody(m_vehicle->m_carChassis, offset));
        m_exhaustSources.push_back(new RenderParticleSource(&m_exhaustShader, renderManager->quad, m_exhausParticles[i], m_linkExhausts[i]));
        renderManager->addParticleSource(m_exhaustSources[i]);
    }

    // tire smoke
    for (int i = 0; i < 4; i++)
    {
        // TODO: use wheel link with offset
        m_linkTireSmokes.push_back(new TransformLinkRigidBody(m_vehicle->m_carChassis, offset));
        RenderParticleSource *renderSource = new RenderParticleSource(&m_tireSmokeShader, m_models.smokeParticleModel,
                                                                      m_tireSmokeParticles[i], m_linkTireSmokes[i]);
        m_tireSmokeSources.push_back(renderSource);
        renderManager->addParticleSource(renderSource);
    }
}

CarController::~CarController()
{
    for (int i = 0; i < m_exhaustCount; i++)
        delete m_exhausParticles[i];
    for (int i = 0; i < 4; i++)
        delete m_tireSmokeParticles[i];
}

void CarController::keyListener(GLFWwindow *window, int key, int scancode, int action, int mods)
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
    else if ((key == GLFW_KEY_3 || key == GLFW_KEY_4) && action == GLFW_PRESS)
    {
        int doorIndex = key == GLFW_KEY_3 ? 0 : 1;
        if (m_vehicle->m_doors[doorIndex].hingeState == HingeState::active)
            m_vehicle->closeDoor(doorIndex);
        else
            m_vehicle->openDoor(doorIndex);
    }
}

void CarController::update(float deltaTime)
{
    m_vehicle->update(deltaTime);
    if (m_controlVehicle)
    {
        // TODO: better?
        m_vehicle->m_controlState.forward = glfwGetKey(m_window, m_keyForward) == GLFW_PRESS;
        m_vehicle->m_controlState.back = glfwGetKey(m_window, m_keyBack) == GLFW_PRESS;
        m_vehicle->m_controlState.left = glfwGetKey(m_window, m_keyLeft) == GLFW_PRESS;
        m_vehicle->m_controlState.right = glfwGetKey(m_window, m_keyRight) == GLFW_PRESS;
        m_vehicle->m_controlState.handbreak = glfwGetKey(m_window, m_keySpace) == GLFW_PRESS;
    }

    for (int i = 0; i < m_exhaustCount; i++)
        updateExhaust(i, deltaTime);

    for (int i = 0; i < 4; i++)
        updateTireSmoke(i, deltaTime);

    followCar(deltaTime);
}

void CarController::followCar(float deltaTime)
{
    if (!m_followVehicle)
        return;

    glm::vec2 p0(0, 0);
    glm::vec2 p1(0.75, 0);
    glm::vec2 p2(0.55, 1);
    glm::vec2 p3(1, 1);
    float cubicBlend = CommonUtil::cubicBezier(p0, p1, p2, p3, m_vehicle->m_speedRate).y;

    // udpate follow specs
    Follow &follow = m_follow;
    follow.stretchTarget = follow.stretchMax * cubicBlend;
    follow.stretchTarget += std::fabs(m_vehicle->gVehicleSteering) * follow.stretchSteeringFactor * (1.f - cubicBlend);
    float lerpFactor = std::clamp(follow.stretchSpeed * deltaTime, 0.f, 1.f);
    follow.stretch = CommonUtil::lerp(follow.stretch, follow.stretchTarget, lerpFactor);

    m_pos = CommonUtil::positionFromModel(m_vehicle->m_chassisModel);
    m_followCamera->position = m_pos - m_followCamera->front * glm::vec3(follow.distance + follow.stretch) + follow.offset;

    if (m_followCamera->moving)
        return;

    glm::vec3 xzTarget;
    xzTarget.x = m_vehicle->m_vehicle->getForwardVector().getX();
    xzTarget.z = m_vehicle->m_vehicle->getForwardVector().getZ();
    float yTarget = m_vehicle->m_vehicle->getForwardVector().getY();

    if (m_vehicle->m_vehicle->getCurrentSpeedKmHour() < 0.f)
        xzTarget *= -1.f;

    // TODO: early exit?

    yTarget -= follow.yOffset;
    yTarget += follow.yStretchFactor * cubicBlend;

    // TODO: deltaTime
    float slerpFactor = std::clamp(cubicBlend * follow.angularSpeed, 0.f, 1.f);
    glm::vec3 xzFront = glm::vec3(m_followCamera->front.x, 0.f, m_followCamera->front.z);
    glm::vec3 xzResult = glm::slerp(xzFront, xzTarget, slerpFactor);

    m_followCamera->front.x = xzResult.x;
    m_followCamera->front.z = xzResult.z;
    m_followCamera->front.y = glm::mix(m_followCamera->front.y, yTarget, slerpFactor);

    m_followCamera->front = glm::normalize(m_followCamera->front);

    m_followCamera->up = m_followCamera->worldUp;
    m_followCamera->right = glm::cross(m_followCamera->front, m_followCamera->up);
}

// TODO: sync with engine force
void CarController::updateExhaust(int index, float deltaTime)
{
    m_exhausParticles[index]->update(deltaTime);

    float maxParticlesPerSecond = 250.0f;
    float maxSpeed = 25.0f;

    float particlesPerSecond = 0.0f;
    if (std::abs(m_vehicle->m_vehicle->getCurrentSpeedKmHour()) <= maxSpeed)
        particlesPerSecond = (1.f - m_vehicle->m_speed / maxSpeed) * maxParticlesPerSecond;

    m_exhausParticles[index]->m_particlesPerSecond = particlesPerSecond;
}

void CarController::updateTireSmoke(int index, float deltaTime)
{
    bool front = index < 2;
    ParticleEngine *tireSmoke = m_tireSmokeParticles[index];
    tireSmoke->update(deltaTime);

    if (!m_vehicle->m_wheelInContact[index])
    {
        tireSmoke->m_particlesPerSecond = 0.f;
        return;
    }

    float maxParticlesPerSecond = front ? 5.f : 20.f;

    float lateralMove = std::abs(m_vehicle->m_localVelocity.x);
    float particlesPerSecond = lateralMove * maxParticlesPerSecond;

    tireSmoke->m_particlesPerSecond = particlesPerSecond;
}

glm::mat4 CarController::translateOffset(glm::vec3 offset)
{
    glm::mat4 model = m_vehicle->m_chassisModel;
    model = glm::translate(model, offset);

    eTransform transform = eTransform(glm::vec3(0.f, 0.f, 0.f), glm::quat(0.707f, 0.f, -0.707f, 0.f), glm::vec3(0.028f));
    model = model * transform.getModelMatrix();

    return model;
}
