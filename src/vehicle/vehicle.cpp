#include "vehicle.h"

Vehicle::Vehicle(PhysicsWorld *physicsWorld, btVector3 position)
    : m_physicsWorld(physicsWorld),
      m_position(position)
{
    this->initDefaultValues();
    this->initVehicle();
}

Vehicle::~Vehicle()
{
    delete m_vehicleRayCaster;
    delete m_vehicle;
}

void Vehicle::initDefaultValues()
{
    this->gEngineForce = 0.f;
    this->accelerationVelocity = 5000.f;
    this->decreaseVelocity = 3000.f;
    this->breakingVelocity = 100.f;

    this->maxEngineForce = 6000.f;  // this should be engine/velocity dependent
    this->minEngineForce = -3000.f; // this should be engine/velocity dependent

    this->gVehicleSteering = 0.f;
    this->steeringSpeed = 0.15f;
    this->steeringIncrement = 2.f;
    this->steeringClamp = 0.5f;

    this->wheelRadius = 0.6f;

    // WheelInfo
    m_wheelFriction = 4000;
    m_suspensionStiffness = 20.f;
    m_suspensionDamping = 2.3f;
    m_suspensionCompression = 4.4f;
    m_rollInfluence = 0.1f;
    m_suspensionRestLength = 0.75f;
}

void Vehicle::initVehicle()
{
    // TODO: get from physicsWorld
    btCollisionShape *chassisShape = new btBoxShape(btVector3(1.f, 0.5f, 2.4f));
    // m_collisionShapes.push_back(chassisShape);

    // TODO: get from physicsWorld
    btCompoundShape *compound = new btCompoundShape();
    // m_collisionShapes.push_back(compound);

    btTransform localTrans;
    localTrans.setIdentity();
    // localTrans effectively shifts the center of mass with respect to the chassis
    localTrans.setOrigin(btVector3(0, 1.0f, 0));

    compound->addChildShape(localTrans, chassisShape);

    btTransform tr;
    tr.setIdentity();
    tr.setOrigin(m_position);

    const btScalar chassisMass = 1000.f;
    m_carChassis = m_physicsWorld->createRigidBody(compound, chassisMass, tr.getOrigin());
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
}

void Vehicle::resetVehicle(btTransform tr)
{
    m_carChassis->setCenterOfMassTransform(tr);
    m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
    m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
    m_physicsWorld->m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_carChassis->getBroadphaseHandle(), m_physicsWorld->m_dynamicsWorld->getDispatcher());
}

void Vehicle::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(m_position);
        resetVehicle(tr);
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(m_carChassis->getWorldTransform().getOrigin() + btVector3(0, 5, 0));
        resetVehicle(tr);
    }
}

void Vehicle::staticKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Vehicle *v = (Vehicle *)glfwGetWindowUserPointer(window);
    v->keyCallback(window, key, scancode, action, mods);
}

void Vehicle::update(GLFWwindow *window, float deltaTime)
{
    m_speed = m_carChassis->getLinearVelocity().length();
    updateSteering(window, deltaTime);
    updateAcceleration(window, deltaTime);
}

// TODO: steeringIncrement based on vehicle velocity
void Vehicle::updateSteering(GLFWwindow *window, float deltaTime)
{
    float steeringDelta = steeringIncrement * deltaTime;

    if (glfwGetKey(window, m_keyLeft) != GLFW_RELEASE)
    {
        gVehicleSteering -= steeringDelta;
        if (gVehicleSteering < -steeringClamp)
            gVehicleSteering = -steeringClamp;
    }
    if (glfwGetKey(window, m_keyRight) != GLFW_RELEASE)
    {
        gVehicleSteering += steeringDelta;
        if (gVehicleSteering > steeringClamp)
            gVehicleSteering = steeringClamp;
    }
    if (glfwGetKey(window, m_keyRight) == GLFW_RELEASE && glfwGetKey(window, m_keyLeft) == GLFW_RELEASE)
    {
        if (abs(gVehicleSteering) < steeringDelta)
        {
            gVehicleSteering = 0;
        }
        else if (gVehicleSteering > 0)
        {
            gVehicleSteering -= steeringDelta;
        }
        else if (gVehicleSteering < 0)
        {
            gVehicleSteering += steeringDelta;
        }
    }

    float steer = CommonUtil::lerp(m_vehicle->getSteeringValue(0), -gVehicleSteering, steeringSpeed);

    m_vehicle->setSteeringValue(steer, 0);
    m_vehicle->setSteeringValue(steer, 1);
}

// TODO: cubic curve acceleration, transmission
void Vehicle::updateAcceleration(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, m_keyForward) != GLFW_RELEASE)
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

    if (glfwGetKey(window, m_keyBack) != GLFW_RELEASE)
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

    m_inAction = glfwGetKey(window, m_keyForward) != GLFW_RELEASE || glfwGetKey(window, m_keyBack) != GLFW_RELEASE;

    if (gEngineForce >= maxEngineForce)
    {
        gEngineForce = maxEngineForce;
    }
    else if (gEngineForce <= minEngineForce)
    {
        gEngineForce = minEngineForce;
    }

    m_vehicle->applyEngineForce(gEngineForce, 0);
    m_vehicle->applyEngineForce(gEngineForce, 1);

    if ((gEngineForce > 0.f && glfwGetKey(window, m_keyBack) != GLFW_RELEASE) ||
        (gEngineForce < 0.f && glfwGetKey(window, m_keyForward) != GLFW_RELEASE))
    {
        m_vehicle->setBrake(breakingVelocity, 2);
        m_vehicle->setBrake(breakingVelocity, 3);
    }
    else
    {
        m_vehicle->setBrake(0, 2);
        m_vehicle->setBrake(0, 3);
    }
}
