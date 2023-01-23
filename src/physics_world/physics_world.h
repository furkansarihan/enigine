#ifndef physics_world_hpp
#define physics_world_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();
    btDiscreteDynamicsWorld *dynamicsWorld;
    btRigidBody *createBox(const btScalar mass, const btVector3 &size, const btVector3 &position);
    btRigidBody *createSphere(const btScalar mass, const btScalar radius, const btVector3 &position);
    btRigidBody *createTerrain(const int width, const int height, const float* heightfieldData, 
        btScalar minHeight, btScalar maxHeight, int upAxis, bool flipQuadEdges);

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
