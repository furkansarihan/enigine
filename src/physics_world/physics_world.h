#ifndef physics_world_hpp
#define physics_world_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "btBulletDynamicsCommon.h"

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();
    btDiscreteDynamicsWorld *dynamicsWorld;
    btRigidBody *getBoxBody(const btScalar mass, const btVector3 &size, const btVector3 &position);
    btRigidBody *getSphereBody(const btScalar mass, const btScalar radius, const btVector3 &position);

private:
    void init();
    btRigidBody *createRigidBody(btCollisionShape *shape, const btScalar mass, const btVector3 &position);
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *overlappingPairCache;
    btSequentialImpulseConstraintSolver *solver;
    btAlignedObjectArray<btCollisionShape *> collisionShapes;
};

#endif /* physics_world_hpp */
