#include "physics_world.h"

PhysicsWorld::PhysicsWorld()
{
    init();
}

PhysicsWorld::~PhysicsWorld()
{
    // remove the rigidbodies from the dynamics world and delete them
    for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject *obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody *body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        m_dynamicsWorld->removeCollisionObject(obj);

        btCollisionShape *shape = body->getCollisionShape();
        delete obj;

        // Check if the shape is a compound shape
        if (btCompoundShape *compoundShape = dynamic_cast<btCompoundShape *>(shape))
        {
            // Iterate through each child shape of the compound shape and delete them
            int numChildShapes = compoundShape->getNumChildShapes();
            for (int j = 0; j < numChildShapes; ++j)
            {
                btCollisionShape *childShape = compoundShape->getChildShape(j);
                delete childShape;
            }
        }

        // Delete the collision shape
        delete shape;
    }

    delete m_dynamicsWorld;
    delete m_solver;
    delete m_overlappingPairCache;
    delete m_dispatcher;
    delete m_collisionConfiguration;
}

void PhysicsWorld::init()
{
    m_useMCLPSolver = false;
    m_useSoftBodyWorld = false;

    // collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();

    // use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

    // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    m_overlappingPairCache = new btDbvtBroadphase();

    if (m_useMCLPSolver)
    {
        btDantzigSolver *mlcp = new btDantzigSolver();
        // btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel();
        m_solver = new btMLCPSolver(mlcp);
    }
    else
    {
        // the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        m_solver = new btSequentialImpulseConstraintSolver();
    }

    if (m_useSoftBodyWorld)
    {
        btSoftBodySolver *softBodySolver = 0;
        m_dynamicsWorld = new btSoftRigidDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration, softBodySolver);

        m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
    }
    else
    {
        m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
    }

    if (m_useMCLPSolver)
    {
        m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 1; // for direct solver it is better to have a small A matrix
    }
    else
    {
        m_dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128; // for direct solver, it is better to solve multiple objects together, small batches have high overhead
    }

    // default gravity
    m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void PhysicsWorld::update(float deltaTime)
{
    m_dynamicsWorld->stepSimulation(deltaTime, m_maxSubSteps);
    if (m_dynamicsWorld->getConstraintSolver()->getSolverType() == BT_MLCP_SOLVER)
    {
        btMLCPSolver *sol = (btMLCPSolver *)m_dynamicsWorld->getConstraintSolver();
        int numFallbacks = sol->getNumFallbacks();
        if (numFallbacks)
        {
            static int totalFailures = 0;
            totalFailures += numFallbacks;
            printf("MLCP solver failed %d times, falling back to btSequentialImpulseSolver (SI)\n", totalFailures);
        }
        sol->setNumFallbacks(0);
    }
}

btSoftRigidDynamicsWorld *PhysicsWorld::softDynamicsWorld()
{
    btSoftRigidDynamicsWorld *sdw = dynamic_cast<btSoftRigidDynamicsWorld *>(m_dynamicsWorld);
    return sdw;
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createBox(const btScalar mass, const btVector3 &size, const btVector3 &position)
{
    btCollisionShape *shape = new btBoxShape(size);
    return createRigidBody(shape, mass, position);
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createSphere(const btScalar mass, const btScalar radius, const btVector3 &position)
{
    btCollisionShape *shape = new btSphereShape(radius);
    return createRigidBody(shape, mass, position);
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createCapsule(const btScalar mass, const btScalar axis, const btScalar radius, const btScalar height, const btVector3 &position)
{
    btCollisionShape *shape;

    if (axis == 0)
    {
        shape = new btCapsuleShapeX(radius, height);
    }
    else if (axis == 2)
    {
        shape = new btCapsuleShapeZ(radius, height);
    }
    else
    {
        shape = new btCapsuleShape(radius, height);
    }

    return createRigidBody(shape, mass, position);
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createCylinder(const btScalar mass, const btScalar axis, const btVector3 &halfExtend, const btVector3 &position)
{
    btCollisionShape *shape;

    if (axis == 0)
    {
        shape = new btCylinderShapeX(halfExtend);
    }
    else if (axis == 2)
    {
        shape = new btCylinderShapeZ(halfExtend);
    }
    else
    {
        shape = new btCylinderShape(halfExtend);
    }

    return createRigidBody(shape, mass, position);
}

btRigidBody *PhysicsWorld::createTerrain(const int width, const int height, const float *heightfieldData,
                                         btScalar minHeight, btScalar maxHeight, int upAxis, bool flipQuadEdges)
{
    btHeightfieldTerrainShape *shape = new btHeightfieldTerrainShape(
        width,
        height,
        heightfieldData,
        minHeight,
        maxHeight,
        upAxis,
        flipQuadEdges);

    shape->setUseDiamondSubdivision(true);
    // shape->setUseZigzagSubdivision(true);

    // TODO: scale the shape
    // TODO: position

    return createRigidBody(shape, 0, btVector3(0, 0, 0));
}

btRigidBody *PhysicsWorld::createRigidBody(btCollisionShape *shape, const btScalar mass, const btVector3 &position)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);

    return createRigidBody(shape, mass, transform);
}

btRigidBody *PhysicsWorld::createRigidBody(btCollisionShape *shape, const btScalar mass, const btTransform &transform)
{
    bool isDynamic = (mass != 0.f);
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        shape->calculateLocalInertia(mass, localInertia);
    // using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState *myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, myMotionState, shape, localInertia);
    btRigidBody *rigidBody = new btRigidBody(RigidBodyCI);
    m_dynamicsWorld->addRigidBody(rigidBody);
    return rigidBody;
}
