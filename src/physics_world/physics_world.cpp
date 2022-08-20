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
    // collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    this->collisionConfiguration = new btDefaultCollisionConfiguration();

    // use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);

    // btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    this->overlappingPairCache = new btDbvtBroadphase();

    // the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    this->solver = new btSequentialImpulseConstraintSolver;

    this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    // default gravity
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::getBoxBody(const btScalar mass, const btVector3 &size, const btVector3 &position)
{
    btCollisionShape *shape = new btBoxShape(size);
    return this->createRigidBody(shape, mass, position);
}

// TODO: reuse shapes
btRigidBody *PhysicsWorld::getSphereBody(const btScalar mass, const btScalar radius, const btVector3 &position)
{
    btCollisionShape *shape = new btSphereShape(radius);
    return this->createRigidBody(shape, mass, position);
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