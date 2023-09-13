#ifndef culling_manager_hpp
#define culling_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>

#include "../utils/bullet_glm.h"
#include "../physics_world/debug_drawer/debug_drawer.h"
#include "../camera/camera.h"

struct CulledObject
{
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    void *userPointer;
};

class CullingManager
{
public:
    CullingManager();
    ~CullingManager();

    DebugDrawer *m_debugDrawer;
    btCollisionWorld *m_collisionWorld;

    void setupFrame(glm::mat4 viewProjection);
    btCollisionObject *addObject(void *userPointer, const float radius, const glm::mat4 &modelMatrix);
    btCollisionObject *addObject(void *userPointer, const glm::vec3 &size, const glm::mat4 &modelMatrix);
    void removeObject(void *userPointer);
    void updateObject(void *userPointer, const glm::mat4 &modelMatrix);
    std::vector<CulledObject> getObjects(glm::vec3 aabbMin, glm::vec3 aabbMax, glm::vec3 viewPos);

private:
    btDefaultCollisionConfiguration *m_collisionConfiguration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_broadphase;

    std::map<void *, btCollisionObject *> m_collisionObjects;

    void init();
    btCollisionObject *createCollisionObject(void *id, btCollisionShape *shape, const glm::mat4 &modelMatrix);

    // frustum culling
    glm::vec4 m_planes[6];
    bool inFrustum(glm::vec3 aabbMin, glm::vec3 aabbMax, glm::vec3 viewPos);
    void calculatePlanes(glm::mat4 viewProjection);
};

#endif /* culling_manager_hpp */
