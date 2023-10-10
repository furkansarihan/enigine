#include "car_controller.h"

CarController::CarController(GLFWwindow *window, ShaderManager *shaderManager, RenderManager *renderManager, PhysicsWorld *physicsWorld, ResourceManager *resourceManager, Camera *followCamera, glm::vec3 position)
    : m_window(window),
      m_followCamera(followCamera)
{
    m_vehicle = new Vehicle(physicsWorld, resourceManager, position);
    m_vehicle->m_carChassis->setUserPointer(this);

    m_follow.offset = glm::vec3(0.0f, 2.5f, 0.0f);
    m_follow.distance = 15.0f;
    m_follow.stretch = 0.0f;
    m_follow.stretchTarget = 0.0f;
    m_follow.stretchMax = 6.f;
    m_follow.stretchSpeed = 0.75f;
    m_follow.stretchSteeringFactor = 4.0f;
    m_follow.yOffset = 0.2f;
    m_follow.yStretchFactor = 0.1f;
    m_follow.angularSpeed = 0.05f;

    m_exhausParticle = new ParticleEngine(followCamera);
    m_exhausParticle->m_particlesPerSecond = 100.f;
    m_exhausParticle->m_randomness = 0.5f;
    m_exhausParticle->m_minVelocity = 0.1f;
    m_exhausParticle->m_maxVelocity = 0.35f;
    m_exhausParticle->m_minDuration = 1.0f;
    m_exhausParticle->m_maxDuration = 2.0f;
    m_exhausParticle->m_particleScale = 0.1f;

    // TODO: smoke textured particles - no particle sorting
    for (int i = 0; i < 4; i++)
    {
        bool front = i < 2;
        ParticleEngine *particle = new ParticleEngine(followCamera);
        particle->m_particlesPerSecond = 0.f;
        particle->m_randomness = 1.f;
        particle->m_minVelocity = front ? 0.5f : 1.f;
        particle->m_maxVelocity = front ? 1.f : 3.f;
        particle->m_minDuration = 1.f;
        particle->m_maxDuration = 2.f;
        particle->m_particleScale = front ? 0.75f : 3.f;

        m_tireSmokeParticles.push_back(particle);
    }

    shaderManager->addShader(ShaderDynamic(&m_exhaustShader, "assets/shaders/smoke.vs", "assets/shaders/exhaust.fs"));
    shaderManager->addShader(ShaderDynamic(&m_tireSmokeShader, "assets/shaders/smoke.vs", "assets/shaders/tire_smoke.fs"));

    // TODO: variable path
    // models
    m_models.carBody = resourceManager->getModel("assets/car/body.gltf", false);
    m_models.carHood = resourceManager->getModel("assets/car/hood.gltf", false);
    m_models.carTrunk = resourceManager->getModel("assets/car/trunk.gltf", false);
    // TODO: single wheel with rotation offset transform
    m_models.wheelModels[0] = m_models.carWheelFL = resourceManager->getModel("assets/car/wheel-fl.gltf", false);
    m_models.wheelModels[1] = m_models.carWheelFR = resourceManager->getModel("assets/car/wheel-fr.gltf", false);
    m_models.wheelModels[2] = m_models.carWheelRL = resourceManager->getModel("assets/car/wheel-rl.gltf", false);
    m_models.wheelModels[3] = m_models.carWheelRR = resourceManager->getModel("assets/car/wheel-rr.gltf", false);
    m_models.doorModels[0] = m_models.carDoorFL = resourceManager->getModel("assets/car/door-fl.gltf", false);
    m_models.doorModels[1] = m_models.carDoorFR = resourceManager->getModel("assets/car/door-fr.gltf", false);
    m_models.doorModels[2] = m_models.carDoorRL = resourceManager->getModel("assets/car/door-rl.gltf", false);
    m_models.doorModels[3] = m_models.carDoorRR = resourceManager->getModel("assets/car/door-rr.gltf", false);

    eTransform transform = eTransform(glm::vec3(0.f, 2.07f, -0.14f), glm::quat(0.707f, 0.f, -0.707f, 0.f), glm::vec3(0.028f));
    TransformLinkRigidBody *linkBody = new TransformLinkRigidBody(m_vehicle->m_carChassis, transform);

    transform.setPosition(glm::vec3(0.f, 1.933f, 3.112f));
    TransformLinkRigidBody *linkHood = new TransformLinkRigidBody(m_vehicle->m_carChassis, transform);

    transform.setPosition(glm::vec3(0.f, 2.07f, -3.89f));
    TransformLinkRigidBody *linkTrunk = new TransformLinkRigidBody(m_vehicle->m_carChassis, transform);

    m_bodySource = RenderSourceBuilder()
                       .setModel(m_models.carBody)
                       .setTransformLink(linkBody)
                       .setAoRoughMetalMap(true)
                       .build();
    renderManager->addSource(m_bodySource);

    m_bodyHood = RenderSourceBuilder()
                     .setModel(m_models.carHood)
                     .setTransformLink(linkHood)
                     .setAoRoughMetalMap(true)
                     .build();
    renderManager->addSource(m_bodyHood);

    m_bodyTrunk = RenderSourceBuilder()
                      .setModel(m_models.carTrunk)
                      .setTransformLink(linkTrunk)
                      .setAoRoughMetalMap(true)
                      .build();
    renderManager->addSource(m_bodyTrunk);

    transform.setPosition(glm::vec3(0.f, 0.f, 0.f));
    for (int i = 0; i < 4; i++)
    {
        TransformLinkWheel *link = new TransformLinkWheel(m_vehicle->m_vehicle, i, transform);

        m_wheelSources[i] = RenderSourceBuilder()
                                .setModel(m_models.wheelModels[i])
                                .setTransformLink(link)
                                .setAoRoughMetalMap(true)
                                .build();
        renderManager->addSource(m_wheelSources[i]);
    }

    eTransform transformActive = transform;
    transformActive.setRotation(glm::quat(0.707f, 0.707f, 0.f, 0.f));
    for (int i = 0; i < 4; i++)
    {
        transform.setPosition(BulletGLM::getGLMVec3(m_vehicle->m_doors[i].posOffset));
        TransformLinkDoor *link = new TransformLinkDoor(m_vehicle, i, transformActive, transform);

        m_doorSources[i] = RenderSourceBuilder()
                               .setModel(m_models.doorModels[i])
                               .setTransformLink(link)
                               .setAoRoughMetalMap(true)
                               .build();
        renderManager->addSource(m_doorSources[i]);
    }

    // exhaust
    transform = eTransform(glm::vec3(0.98f, 0.93f, -4.34f), glm::quat(0.381f, -0.186f, -0.813f, 0.394f), glm::vec3(1.f));
    TransformLinkRigidBody *linkExhaust = new TransformLinkRigidBody(m_vehicle->m_carChassis, transform);
    m_exhaustSource = new RenderParticleSource(&m_exhaustShader, m_exhausParticle, linkExhaust);
    renderManager->addParticleSource(m_exhaustSource);

    // tire smoke
    float wheelGap = 0.2f;
    glm::vec3 wheelPos[4] = {
        glm::vec3(-1.5, wheelGap, 2.5),
        glm::vec3(1.5, wheelGap, 2.5),
        glm::vec3(1.5, wheelGap, -2.5),
        glm::vec3(-1.5, wheelGap, -2.5)};

    for (int i = 0; i < 4; i++)
    {
        transform = eTransform(wheelPos[i], glm::quat(0.381f, -0.186f, -0.813f, 0.394f), glm::vec3(1.f));
        TransformLinkRigidBody *linkTireSmoke = new TransformLinkRigidBody(m_vehicle->m_carChassis, transform);
        RenderParticleSource *renderSource = new RenderParticleSource(&m_tireSmokeShader, m_tireSmokeParticles[i], linkTireSmoke);
        m_tireSmokeSources.push_back(renderSource);
        renderManager->addParticleSource(renderSource);
    }
}

CarController::~CarController()
{
    delete m_vehicle;
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

    updateExhaust(deltaTime);
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

void CarController::updateExhaust(float deltaTime)
{
    m_exhausParticle->update(deltaTime);

    float maxParticlesPerSecond = 250.0f;
    float maxSpeed = 10.0f;

    float particlesPerSecond = 0.0f;
    if (m_vehicle->m_speed <= maxSpeed)
        particlesPerSecond = (1.f - m_vehicle->m_speed / maxSpeed) * maxParticlesPerSecond;

    m_exhausParticle->m_particlesPerSecond = particlesPerSecond;
}

// TODO: check if tire on the ground
void CarController::updateTireSmoke(int index, float deltaTime)
{
    bool front = index < 2;
    ParticleEngine *tireSmoke = m_tireSmokeParticles[index];
    tireSmoke->update(deltaTime);

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
