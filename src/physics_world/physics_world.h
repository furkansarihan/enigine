#ifndef physics_world_hpp
#define physics_world_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"

class PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    btDiscreteDynamicsWorld *m_dynamicsWorld;
    int m_maxSubSteps = 1;

    btRigidBody *createBox(const btScalar mass, const btVector3 &size, const btVector3 &position);
    btRigidBody *createSphere(const btScalar mass, const btScalar radius, const btVector3 &position);
    btRigidBody *createCylinder(const btScalar mass, const btScalar axis, const btVector3 &halfExtend, const btVector3 &position);
    btRigidBody *createCapsule(const btScalar mass, const btScalar axis, const btScalar radius, const btScalar height, const btVector3 &position);
    btRigidBody *createTerrain(const int width, const int height, const float *heightfieldData,
                               btScalar minHeight, btScalar maxHeight, int upAxis, bool flipQuadEdges);
    btRigidBody *createRigidBody(btCollisionShape *shape, const btScalar mass, const btVector3 &position);
    btRigidBody *createRigidBody(btCollisionShape *shape, const btScalar mass, const btTransform &transform);

    void update(float deltaTime);
    btSoftRigidDynamicsWorld *softDynamicsWorld();

    // TODO: freeze rigidbody

private:
    bool m_useMCLPSolver;
    bool m_useSoftBodyWorld;
    btDefaultCollisionConfiguration *m_collisionConfiguration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_overlappingPairCache;
    btSequentialImpulseConstraintSolver *m_solver;
    btAlignedObjectArray<btCollisionShape *> m_collisionShapes;

    void init();
};

#endif /* physics_world_hpp */
