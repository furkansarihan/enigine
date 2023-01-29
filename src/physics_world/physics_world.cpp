#include "physics_world.h"

PhysicsWorld::PhysicsWorld()
{
    this->init();
}

PhysicsWorld::~PhysicsWorld()
{
    // remove the rigidbodies from the dynamics world and delete them
    for (int i = this->dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject *obj = this->dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody *body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        this->dynamicsWorld->removeCollisionObject(obj);
        delete obj;
        // TODO: refactor when shape reuse impl
        if (body && body->getCollisionShape())
        {
            delete body->getCollisionShape();
        }
    }

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;
}

void PhysicsWorld::init()
{
    this->useMCLPSolver = false;

    // collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    this->collisionConfiguration = new btDefaultCollisionConfiguration();

    // use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    this->overlappingPairCache = new btDbvtBroadphase();

    if (useMCLPSolver)
    {
        btDantzigSolver *mlcp = new btDantzigSolver();
        // btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel();
        this->solver = new btMLCPSolver(mlcp);
    }
    else
    {
        // the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        this->solver = new btSequentialImpulseConstraintSolver();
    }

    this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    if (useMCLPSolver)
    {
        this->dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 1; // for direct solver it is better to have a small A matrix
    }
    else
    {
        this->dynamicsWorld->getSolverInfo().m_minimumSolverBatchSize = 128; // for direct solver, it is better to solve multiple objects together, small batches have high overhead
    }

    // default gravity
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createBox(const btScalar mass, const btVector3 &size, const btVector3 &position)
{
    btCollisionShape *shape = new btBoxShape(size);
    return this->createRigidBody(shape, mass, position);
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::createSphere(const btScalar mass, const btScalar radius, const btVector3 &position)
{
    btCollisionShape *shape = new btSphereShape(radius);
    return this->createRigidBody(shape, mass, position);
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

    // TODO: scale the shape
    // TODO: position

    return this->createRigidBody(shape, 0, btVector3(0, 0, 0));
}

btRigidBody *PhysicsWorld::createRigidBody(btCollisionShape *shape, const btScalar mass, const btVector3 &position)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);
    bool isDynamic = (mass != 0.f);
    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
        shape->calculateLocalInertia(mass, localInertia);
    // using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState *myMotionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, myMotionState, shape, localInertia);
    btRigidBody *rigidBody = new btRigidBody(RigidBodyCI);
    this->dynamicsWorld->addRigidBody(rigidBody);
    return rigidBody;
}
