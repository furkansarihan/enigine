#ifndef culling_manager_hpp
#define culling_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include "btBulletDynamicsCommon.h"

class CullingManager
{
public:
    CullingManager();
    ~CullingManager();

    btCollisionObject *addObject(void *userPointer, const btScalar radius, const btVector3 &position);
    void updateObject(void *userPointer, const btVector3 &position);
    std::vector<void *> getObjects(btVector3 aabbMin, btVector3 aabbMax);

private:
    btCollisionWorld *collisionWorld;
    btDefaultCollisionConfiguration *collisionConfiguration;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *broadphase;

    std::map<void *, btCollisionObject *> collisionObjects;

    void init();
    btCollisionObject *createCollisionObject(void *id, btCollisionShape *shape, const btVector3 &position);
};

#endif /* culling_manager_hpp */
