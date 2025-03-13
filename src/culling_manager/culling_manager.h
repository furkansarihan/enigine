#ifndef culling_manager_hpp
#define culling_manager_hpp

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>

#include "../camera/camera.h"
#include "../physics_world/debug_drawer/debug_drawer.h"
#include "../utils/bullet_glm.h"

struct SelectedObject
{
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    glm::vec3 hitPointWorld;
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
    std::vector<SelectedObject> getObjects(glm::vec3 rayFrom, glm::vec3 rayTo);
    std::vector<SelectedObject> getObjects(glm::vec3 aabbMin, glm::vec3 aabbMax, glm::vec3 viewPos);

    // frustum culling
    glm::vec4 m_planes[6];
    void calculatePlanes(glm::mat4 viewProjection);
    bool inFrustum(const glm::vec3 &aabbMin, const glm::vec3 &aabbMax, const glm::vec3 &viewPos);

private:
    btDefaultCollisionConfiguration *m_collisionConfiguration;
    btCollisionDispatcher *m_dispatcher;
    btBroadphaseInterface *m_broadphase;

    std::map<void *, btCollisionObject *> m_collisionObjects;

    void init();
    btCollisionObject *createCollisionObject(void *id, btCollisionShape *shape, const glm::mat4 &modelMatrix);
};

#endif /* culling_manager_hpp */
