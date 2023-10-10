#include "vehicle.h"

#include <glm/gtc/matrix_inverse.hpp>

Vehicle::Vehicle(PhysicsWorld *physicsWorld, ResourceManager *resourceManager, glm::vec3 position)
    : m_physicsWorld(physicsWorld),
      m_resourceManager(resourceManager),
      m_position(position)
{
    initDefaultValues();
    initVehicle();
}

Vehicle::~Vehicle()
{
    delete m_vehicleRayCaster;
    delete m_vehicle;
}

void Vehicle::initDefaultValues()
{
    gEngineForce = 0.f;
    accelerationVelocity = 10000.f;
    decreaseVelocity = 30000.f;
    breakingVelocity = 10000.f;
    handBreakingVelocity = 25000.f;
    maxEngineForce = 10000.f;
    minEngineForce = -10000.f;
    gVehicleSteering = 0.f;
    steeringSpeed = 0.3f;
    steeringIncrement = 20.f;
    steeringLimit = 0.8f;
    wheelRadius = 0.6f;
    m_maxSteerSpeed = 80.f;
    m_returnIdleFactor = 2.f;

    // WheelInfo
    m_wheelFriction = 5.f;
    m_driftFriction = 3.f;
    m_suspensionStiffness = 20.f;
    m_suspensionDamping = 3.f;
    m_suspensionCompression = 0.5f;
    m_rollInfluence = 0.5f;
    m_suspensionRestLength = 0.75f;

    // doors
    m_doors[0].aFrame = btVector3(1.5f, 2.9f, 1.6f);
    m_doors[1].aFrame = btVector3(-1.5f, 2.9f, 1.6f);
    m_doors[2].aFrame = btVector3(1.5f, 2.9f, -0.6f);
    m_doors[3].aFrame = btVector3(-1.5f, 2.9f, -0.6f);

    m_doors[0].bFrame = btVector3(0.98f, -0.01f, 0.95f);
    m_doors[1].bFrame = btVector3(0.98f, -0.04f, 0.95f);
    m_doors[2].bFrame = btVector3(0.728f, 0.03f, 0.89f);
    m_doors[3].bFrame = btVector3(0.728f, -0.04f, 0.89f);

    m_doors[0].posOffset = btVector3(1.51f, 1.95f, 0.62);
    m_doors[1].posOffset = btVector3(-1.46f, 1.95f, 0.62);
    m_doors[2].posOffset = btVector3(1.46f, 2.01f, -1.33);
    m_doors[3].posOffset = btVector3(-1.46f, 2.01f, -1.32);
}

void Vehicle::initVehicle()
{
    setupCollider();

    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(BulletGLM::getBulletVec3(m_position));

    const btScalar chassisMass = 1000.f;
    m_carChassis = m_physicsWorld->createRigidBody(m_compoundShape, chassisMass, tr.getOrigin());
    m_carChassis->setDamping(0.2, 0.2);

    float wheelGap = 1.4f;
    btVector3 wheelPos[4] = {
        btVector3(btScalar(-1.5), btScalar(wheelGap), btScalar(2.5)),
        btVector3(btScalar(1.5), btScalar(wheelGap), btScalar(2.5)),
        btVector3(btScalar(1.5), btScalar(wheelGap), btScalar(-2.5)),
        btVector3(btScalar(-1.5), btScalar(wheelGap), btScalar(-2.5))};

    m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_physicsWorld->m_dynamicsWorld);
    m_vehicle = new btRaycastVehicle(m_tuning, m_carChassis, m_vehicleRayCaster);

    m_carChassis->setActivationState(DISABLE_DEACTIVATION);

    m_physicsWorld->m_dynamicsWorld->addVehicle(m_vehicle);

    int rightIndex = 0;
    int upIndex = 1;
    int forwardIndex = 2;
    m_vehicle->setCoordinateSystem(rightIndex, upIndex, forwardIndex);

    btVector3 wheelDirectionCS0(0, -1, 0);
    btVector3 wheelAxleCS(-1, 0, 0);

    m_vehicle->addWheel(wheelPos[0], wheelDirectionCS0, wheelAxleCS, m_suspensionRestLength, wheelRadius, m_tuning, true);
    m_vehicle->addWheel(wheelPos[1], wheelDirectionCS0, wheelAxleCS, m_suspensionRestLength, wheelRadius, m_tuning, true);
    m_vehicle->addWheel(wheelPos[2], wheelDirectionCS0, wheelAxleCS, m_suspensionRestLength, wheelRadius, m_tuning, false);
    m_vehicle->addWheel(wheelPos[3], wheelDirectionCS0, wheelAxleCS, m_suspensionRestLength, wheelRadius, m_tuning, false);

    for (int i = 0; i < m_vehicle->getNumWheels(); i++)
    {
        btWheelInfo &wheel = m_vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = m_suspensionStiffness;
        wheel.m_wheelsDampingRelaxation = m_suspensionDamping;
        wheel.m_wheelsDampingCompression = m_suspensionCompression;
        wheel.m_frictionSlip = m_wheelFriction;
        wheel.m_rollInfluence = m_rollInfluence;
    }

    setupDoors();
}

// TODO: variable path
void Vehicle::setupCollider()
{
    m_compoundShape = new btCompoundShape();

    m_collider = m_resourceManager->getModel("assets/car/car-collider.obj");

    for (int i = 0; i < m_collider->meshes.size(); i++)
    {
        Mesh &mesh = *m_collider->meshes[i];

        btConvexHullShape *shape = getBodyShape(mesh);

        btTransform localTrans;
        localTrans.setIdentity();
        localTrans.setOrigin(btVector3(0.f, 0.3f, -0.35f));
        // Same with car rotation
        localTrans.setRotation(btQuaternion(0.f, -0.707f, 0.f, 0.707f));

        m_compoundShape->addChildShape(localTrans, shape);
    }
}

btConvexHullShape *Vehicle::getBodyShape(Mesh &mesh)
{
    btConvexHullShape *convexShape = new btConvexHullShape();

    for (int i = 0; i < mesh.vertices.size(); ++i)
        convexShape->addPoint(BulletGLM::getBulletVec3(mesh.vertices[i].position));

    // Optional: Enable margin for better collision detection
    convexShape->setMargin(0.04f); // Set an appropriate margin value
    // Same with car scale
    convexShape->setLocalScaling(btVector3(0.028f, 0.028f, 0.028f));

    return convexShape;
}

void Vehicle::setupDoors()
{
    btBoxShape *shape = new btBoxShape(btVector3(0.7f, 0.15f, 0.8f));
    float doorMass = 20.f;

    for (int i = 0; i < 4; i++)
    {
        m_doors[i].body = m_physicsWorld->createRigidBody(shape, doorMass, getDoorTransform(i));

        btVector3 yAxis(0, 1, 0);
        btVector3 zAxis(0, 0, 1);
        m_doors[i].joint = new btHingeConstraint(*m_carChassis, *m_doors[i].body, m_doors[i].aFrame, m_doors[i].bFrame, yAxis, zAxis, true);

        if (i % 2 == 0)
            m_doors[i].joint->setLimit(-M_PI_2 - M_PI_4 - M_PI_4 * 0.2f, -M_PI_2);
        else
            m_doors[i].joint->setLimit(-M_PI_2, -M_PI_4 + M_PI_4 * 0.2f);

        m_physicsWorld->m_dynamicsWorld->addConstraint(m_doors[i].joint, true);

        m_doors[i].doorState = DoorState::closed;
        m_doors[i].hingeState = HingeState::active; // for replace logic
        updateHingeState(i, HingeState::deactive);
    }
}

void Vehicle::updateHingeState(int door, HingeState newState)
{
    HingeState currentState = m_doors[door].hingeState;

    if (currentState == newState)
        return;

    if (newState == HingeState::deactive)
    {
        m_doors[door].joint->enableMotor(false);
        m_doors[door].joint->setMaxMotorImpulse(0.f);
        m_physicsWorld->m_dynamicsWorld->removeConstraint(m_doors[door].joint);

        m_doors[door].body->setLinearVelocity(btVector3(0, 0, 0));
        m_doors[door].body->setAngularVelocity(btVector3(0, 0, 0));
        m_physicsWorld->m_dynamicsWorld->removeRigidBody(m_doors[door].body);
    }
    else
    {
        // TODO: fix teleport
        btTransform transform = getDoorTransform(door);
        m_doors[door].body->setWorldTransform(transform);
        // m_doors[door].body->getMotionState()->setWorldTransform(transform);

        m_doors[door].body->setLinearVelocity(btVector3(0, 0, 0));
        m_doors[door].body->setAngularVelocity(btVector3(0, 0, 0));

        m_physicsWorld->m_dynamicsWorld->addRigidBody(m_doors[door].body);

        m_doors[door].joint->enableMotor(true);
        m_doors[door].joint->setMaxMotorImpulse(0.f);
        m_physicsWorld->m_dynamicsWorld->addConstraint(m_doors[door].joint, true);
    }

    m_doors[door].hingeState = newState;
}

btTransform Vehicle::getDoorTransform(int door)
{
    btTransform transform;
    m_carChassis->getMotionState()->getWorldTransform(transform);

    btTransform doorTransform;
    doorTransform.setIdentity();
    doorTransform.setOrigin(m_doors[door].posOffset);
    doorTransform.setRotation(BulletGLM::getBulletQuat(m_doorRotate));

    btTransform bodyTransform;
    bodyTransform.setIdentity();
    // TODO: from car body rotation - fix body rotation align
    bodyTransform.setRotation(BulletGLM::getBulletQuat(glm::quat(glm::vec3(0.f, 0.f, 0.02f))));
    transform = transform * doorTransform * bodyTransform;
    // transform = transform * doorTransform;

    return transform;
}

// TODO: update collider to match door void
void Vehicle::openDoor(int door)
{
    updateHingeState(door, HingeState::active);
    m_doors[door].hingeTarget.angle = door % 2 == 0 ? (float)M_PI : 0.f;
    m_doors[door].doorState = DoorState::open;
}

void Vehicle::closeDoor(int door)
{
    m_doors[door].hingeTarget.angle = door % 2 == 1 ? (float)M_PI : 0.f;
    m_doors[door].doorClosedAt = (float)glfwGetTime();
    m_doors[door].doorState = DoorState::closed;
}

bool Vehicle::isDoorOpen(int door)
{
    return m_doors[door].doorState == DoorState::open;
}

void Vehicle::resetVehicle(btTransform tr)
{
    m_carChassis->setCenterOfMassTransform(tr);
    m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
    m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
    m_physicsWorld->m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_carChassis->getBroadphaseHandle(), m_physicsWorld->m_dynamicsWorld->getDispatcher());
}

void Vehicle::updateDoorAngles(float deltaTime)
{
    for (int i = 0; i < 4; i++)
    {
        if (m_doors[i].hingeState == HingeState::deactive)
            continue;

        if (m_doors[i].doorState == DoorState::closed)
        {
            float now = (float)glfwGetTime();
            float elapsedTime = now - m_doors[i].doorClosedAt;
            float maxTime = 1.f;
            if (elapsedTime > maxTime)
            {
                updateHingeState(i, HingeState::deactive);
                m_doors[i].hingeTarget.angle = 0.f;

                continue;
            }
        }

        m_doors[i].joint->setMotorTarget(m_doors[i].hingeTarget.angle, deltaTime);
        m_doors[i].joint->setMaxMotorImpulse(deltaTime * m_doors[i].hingeTarget.force);
    }
}

glm::vec3 inverseTransformDirection(const glm::mat4 &transformMatrix, const glm::vec3 &direction)
{
    glm::mat4 inverseTransformMatrix = glm::affineInverse(transformMatrix);
    glm::vec4 transformedDir = inverseTransformMatrix * glm::vec4(direction, 0.0f);
    return glm::vec3(transformedDir.x, transformedDir.y, transformedDir.z);
}

void Vehicle::update(float deltaTime)
{
    m_velocity = BulletGLM::getGLMVec3(m_carChassis->getLinearVelocity());
    glm::mat4 transform;
    m_carChassis->getWorldTransform().getOpenGLMatrix((float *)&transform);
    m_localVelocity = inverseTransformDirection(transform, m_velocity);
    m_speed = std::abs(m_velocity.length());

    btTransform chassisTransform;
    m_carChassis->getMotionState()->getWorldTransform(chassisTransform);
    chassisTransform.getOpenGLMatrix((btScalar *)&m_chassisModel);

    for (int i = 0; i < 4; i++)
        m_vehicle->updateWheelTransform(i, true);

    updateSteering(deltaTime);
    updateAcceleration(deltaTime);
    updateDoorAngles(deltaTime);
}

// TODO: steeringIncrement based on vehicle velocity
void Vehicle::updateSteering(float deltaTime)
{
    m_speedZ = std::abs(m_localVelocity.z);
    m_speedRate = std::clamp(m_speedZ / m_maxSteerSpeed, 0.f, 1.f);
    m_speedSteerFactor = CommonUtil::lerp(1.f, 0.2f, m_speedRate);

    float steeringGoal = 0.f;
    if (m_controlState.left)
        steeringGoal += steeringLimit;
    if (m_controlState.right)
        steeringGoal -= steeringLimit;

    if (!m_controlState.left && !m_controlState.right)
    {
        steeringGoal = 0.f;
        m_speedSteerFactor *= m_returnIdleFactor;
    }

    float lerpFactor = steeringIncrement * (m_speedRate + 0.5f) * deltaTime;
    lerpFactor = std::clamp(lerpFactor, 0.f, 1.f);

    gVehicleSteering = CommonUtil::lerp(gVehicleSteering, steeringGoal * m_speedSteerFactor, lerpFactor);

    m_vehicle->setSteeringValue(gVehicleSteering, 0);
    m_vehicle->setSteeringValue(gVehicleSteering, 1);
}

// TODO: cubic curve acceleration, transmission
void Vehicle::updateAcceleration(float deltaTime)
{
    if (m_controlState.forward)
    {
        if (gEngineForce >= 0)
        {
            gEngineForce += accelerationVelocity * deltaTime;
        }
        else
        {
            gEngineForce += breakingVelocity * deltaTime;
        }
    }
    else if (gEngineForce > 0)
    {
        gEngineForce -= decreaseVelocity * deltaTime;
    }

    if (m_controlState.back)
    {
        if (gEngineForce <= 0)
        {
            gEngineForce -= accelerationVelocity * deltaTime;
        }
        else
        {
            gEngineForce -= breakingVelocity * deltaTime;
        }
    }
    else if (gEngineForce < 0)
    {
        gEngineForce += decreaseVelocity * deltaTime;
    }

    m_inAction = m_controlState.forward || m_controlState.back;

    if (!m_inAction)
    {
        // TODO: better stop handling ?
        float velocity = decreaseVelocity * deltaTime;
        if (std::fabs(gEngineForce) < velocity)
        {
            gEngineForce = 0.f;
            m_vehicle->setBrake(100, 0);
            m_vehicle->setBrake(100, 1);
        }
        else
        {
            m_vehicle->setBrake(0, 0);
            m_vehicle->setBrake(0, 1);
        }
    }

    if (gEngineForce >= maxEngineForce)
    {
        gEngineForce = maxEngineForce;
    }
    else if (gEngineForce <= minEngineForce)
    {
        gEngineForce = minEngineForce;
    }

    // TODO: front bias
    m_vehicle->applyEngineForce(gEngineForce, 0);
    m_vehicle->applyEngineForce(gEngineForce, 1);

    if ((gEngineForce > 0.f && m_controlState.back) ||
        (gEngineForce < 0.f && m_controlState.forward))
    {
        for (int i = 0; i < 4; i++)
            m_vehicle->setBrake(breakingVelocity, i);
    }
    else
    {
        for (int i = 0; i < 4; i++)
            m_vehicle->setBrake(0, i);
    }

    if (m_controlState.handbreak)
    {
        m_vehicle->setBrake(handBreakingVelocity, 2);
        m_vehicle->setBrake(handBreakingVelocity, 3);

        // TODO: better way?
        float lateralMove = std::max(0.1f, std::abs(m_localVelocity.x));
        float lateralFactor = std::clamp(1.f / lateralMove, 0.f, 1.f);

        m_vehicle->getWheelInfo(2).m_frictionSlip = m_driftFriction * lateralFactor * m_speedRate;
        m_vehicle->getWheelInfo(3).m_frictionSlip = m_driftFriction * lateralFactor * m_speedRate;
    }
    else
    {
        m_vehicle->getWheelInfo(2).m_frictionSlip = m_wheelFriction;
        m_vehicle->getWheelInfo(3).m_frictionSlip = m_wheelFriction;
    }
}
