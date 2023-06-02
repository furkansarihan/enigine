#include "vehicle.h"

Vehicle::Vehicle(PhysicsWorld *physicsWorld, btVector3 position)
{
    this->physicsWorld = physicsWorld;
    this->position = position;
    this->initDefaultValues();
    this->initVehicle();
}

Vehicle::~Vehicle()
{
}

void Vehicle::initDefaultValues()
{
    this->gEngineForce = 0.f;
    this->accelerationVelocity = 25.f;
    this->decreaseVelocity = 15.f;
    this->breakingVelocity = 40.f;

    this->maxEngineForce = 90.f;  // this should be engine/velocity dependent
    this->minEngineForce = -30.f; // this should be engine/velocity dependent

    this->gVehicleSteering = 0.f;
    this->steeringIncrement = 7.f;
    this->steeringVelocity = 1000.f;
    this->steeringClamp = 0.8f;

    this->wheelRadius = 0.5f;
    this->wheelWidth = 0.4f;

    this->lowerLimit = -1;
    this->upperLimit = 1;
    this->damping = 0.2f;
    this->friction = 2.5f;
    this->stifness = 40.0f;
    this->wheelDamping = 2.0f;
    this->bounce = 0.0f;
}

void Vehicle::initVehicle()
{
    // TODO: get from physicsWorld
    btCollisionShape *chassisShape = new btBoxShape(btVector3(0.8f, 0.3f, 2.8f));
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
    tr.setOrigin(position);

    const btScalar chassisMass = 380.f;
    const btScalar wheelMass = 10.f;
    m_carChassis = physicsWorld->createRigidBody(compound, chassisMass, tr.getOrigin());
    // m_carChassis->setDamping(0.8, 0.8);

    // TODO: get from physicsWorld
    // btCollisionShape *m_wheelShape = new btCylinderShapeX(btVector3(wheelWidth, wheelRadius, wheelRadius));

    float wheelGap = -0.25f;
    btVector3 wheelPos[4] = {
        tr.getOrigin() + btVector3(btScalar(-1.5), btScalar(wheelGap), btScalar(2.5)),
        tr.getOrigin() + btVector3(btScalar(1.5), btScalar(wheelGap), btScalar(2.5)),
        tr.getOrigin() + btVector3(btScalar(1.5), btScalar(wheelGap), btScalar(-2.5)),
        tr.getOrigin() + btVector3(btScalar(-1.5), btScalar(wheelGap), btScalar(-2.5))};

    for (int i = 0; i < 4; i++)
    {
        // create a Hinge2 joint
        // create two rigid bodies
        // static bodyA (parent) on top:

        btRigidBody *pBodyA = this->m_carChassis;
        pBodyA->setActivationState(DISABLE_DEACTIVATION);
        // dynamic bodyB (child) below it :
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(wheelPos[i]);

        btRigidBody *pBodyB;
        pBodyB = physicsWorld->createSphere(wheelMass, wheelRadius, tr.getOrigin());
        // pBodyB = physicsWorld->createRigidBody(m_wheelShape, wheelMass, tr.getOrigin());
        wheelBodies[i] = pBodyB;
        pBodyB->setDamping(damping, damping);
        pBodyB->setFriction(friction);
        pBodyB->setActivationState(DISABLE_DEACTIVATION);
        // add some data to build constraint frames
        btVector3 parentAxis(0.f, 1.f, 0.f);
        btVector3 childAxis(1.f, 0.f, 0.f);
        btVector3 anchor = tr.getOrigin();
        wheels[i] = new btHinge2Constraint(*pBodyA, *pBodyB, anchor, parentAxis, childAxis);

        // add constraint to world
        physicsWorld->dynamicsWorld->addConstraint(wheels[i], true);

        // Drive engine
        wheels[i]->enableMotor(3, true);
        wheels[i]->setMaxMotorForce(3, 1000);
        wheels[i]->setTargetVelocity(3, 0);

        // Steering engine
        wheels[i]->enableMotor(5, true);
        wheels[i]->setMaxMotorForce(5, 1000);
        wheels[i]->setTargetVelocity(5, 0);

        wheels[i]->setParam(BT_CONSTRAINT_CFM, 0.15f, 2);
        wheels[i]->setParam(BT_CONSTRAINT_ERP, 0.35f, 2);

        wheels[i]->setLimit(2, lowerLimit, upperLimit);
        wheels[i]->setBounce(2, bounce);
        wheels[i]->setDamping(2, wheelDamping);
        wheels[i]->setStiffness(2, stifness);
    }
}

void Vehicle::resetVehicle(btTransform tr)
{
    m_carChassis->setCenterOfMassTransform(tr);
    m_carChassis->setLinearVelocity(btVector3(0, 0, 0));
    m_carChassis->setAngularVelocity(btVector3(0, 0, 0));
    physicsWorld->dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(m_carChassis->getBroadphaseHandle(), physicsWorld->dynamicsWorld->getDispatcher());
}

void Vehicle::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(position);
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
    updateSteering(window, deltaTime);
    updateAcceleration(window, deltaTime);
}

// TODO: steeringIncrement based on vehicle velocity
void Vehicle::updateSteering(GLFWwindow *window, float deltaTime)
{
    float steeringDelta = steeringIncrement * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE)
    {
        gVehicleSteering -= steeringDelta;
        if (gVehicleSteering < -steeringClamp)
            gVehicleSteering = -steeringClamp;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE)
    {
        gVehicleSteering += steeringDelta;
        if (gVehicleSteering > steeringClamp)
            gVehicleSteering = steeringClamp;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
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

    for (int i = 0; i < 4; i++)
    {
        float angle = wheels[i]->getAngle(2);
        float distance = (i < 2 ? gVehicleSteering : 0) - angle;

        if (abs(distance) < 0.0001)
        {
            wheels[i]->setTargetVelocity(5, 0);
        }
        else
        {
            glm::vec2 p0(0, 0);
            glm::vec2 p1(0.18, 1);
            glm::vec2 p2(0.57, 1);
            glm::vec2 p3(1, 1);

            wheels[i]->setTargetVelocity(5, deltaTime * CommonUtil::cubicBezier(p0, p1, p2, p3, distance).y * steeringVelocity);
        }
    }
}

// TODO: cubic curve acceleration, transmission
void Vehicle::updateAcceleration(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE)
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

    if (glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE)
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

    if (gEngineForce >= maxEngineForce)
    {
        gEngineForce = maxEngineForce;
    }
    else if (gEngineForce <= minEngineForce)
    {
        gEngineForce = minEngineForce;
    }

    float force = -gEngineForce;

    wheels[0]->setTargetVelocity(3, force);
    wheels[1]->setTargetVelocity(3, force);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        wheels[2]->setTargetVelocity(3, force);
        wheels[3]->setTargetVelocity(3, force);
        return;
    }

    // Handbrake
    glm::vec2 p0(0, 0);
    glm::vec2 p1(0.18, 1);
    glm::vec2 p2(0.57, 1);
    glm::vec2 p3(1, 1);

    float normalized = (gEngineForce - minEngineForce) / (maxEngineForce - minEngineForce);

    wheels[2]->setTargetVelocity(3, deltaTime * -CommonUtil::cubicBezier(p0, p1, p2, p3, normalized).y * breakingVelocity);
    wheels[3]->setTargetVelocity(3, deltaTime * -CommonUtil::cubicBezier(p0, p1, p2, p3, normalized).y * breakingVelocity);
}
