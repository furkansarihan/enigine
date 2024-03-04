#include "culling_manager.h"

#include <algorithm>

class MyOverlapCallback : public btBroadphaseAabbCallback
{
public:
    btAlignedObjectArray<btCollisionObject *> *m_overlappingObjects;

    MyOverlapCallback()
    {
        m_overlappingObjects = new btAlignedObjectArray<btCollisionObject *>();
    }

    ~MyOverlapCallback()
    {
        delete m_overlappingObjects;
    }

    virtual bool process(const btBroadphaseProxy *proxy)
    {
        btCollisionObject *collisionObject = static_cast<btCollisionObject *>(proxy->m_clientObject);
        m_overlappingObjects->push_back(collisionObject);
        return true;
    }
};

CullingManager::CullingManager()
{
    init();
}

CullingManager::~CullingManager()
{
    for (int i = m_collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject *obj = m_collisionWorld->getCollisionObjectArray()[i];

        // TODO: refactor when shape reuse impl
        if (obj && obj->getCollisionShape())
        {
            delete obj->getCollisionShape();
        }

        m_collisionWorld->removeCollisionObject(obj);

        delete obj;
    }

    delete m_dispatcher;
    delete m_broadphase;
    delete m_collisionConfiguration;
    delete m_collisionWorld;
}

void CullingManager::init()
{
    // TODO: minimum runtime ram usage
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_broadphase = new btDbvtBroadphase();
    m_collisionWorld = new btCollisionWorld(m_dispatcher, m_broadphase, m_collisionConfiguration);

    m_debugDrawer = new DebugDrawer();
    m_debugDrawer->setDebugMode(btIDebugDraw::DBG_NoDebug);
    m_collisionWorld->setDebugDrawer(m_debugDrawer);
}

// TODO: reuse shapes
btCollisionObject *CullingManager::addObject(void *userPointer, const float radius, const glm::mat4 &modelMatrix)
{
    btCollisionShape *shape = new btSphereShape(radius);
    return createCollisionObject(userPointer, shape, modelMatrix);
}

btCollisionObject *CullingManager::addObject(void *userPointer, const glm::vec3 &size, const glm::mat4 &modelMatrix)
{
    btCollisionShape *shape = new btBoxShape(BulletGLM::getBulletVec3(size));
    return createCollisionObject(userPointer, shape, modelMatrix);
}

void CullingManager::removeObject(void *userPointer)
{
    btCollisionObject *objectToRemove = nullptr;

    int numCollisionObjects = m_collisionWorld->getNumCollisionObjects();
    for (int i = 0; i < numCollisionObjects; ++i)
    {
        btCollisionObject *obj = m_collisionWorld->getCollisionObjectArray()[i];
        if (obj->getUserPointer() == userPointer)
        {
            objectToRemove = obj;
            break;
        }
    }

    if (objectToRemove)
    {
        if (objectToRemove->getCollisionShape())
            delete objectToRemove->getCollisionShape();

        m_collisionWorld->removeCollisionObject(objectToRemove);

        delete objectToRemove;
    }
}

void CullingManager::updateObject(void *userPointer, const glm::mat4 &modelMatrix)
{
    auto it = m_collisionObjects.find(userPointer);
    if (it != m_collisionObjects.end())
    {
        btTransform transform;
        transform.setFromOpenGLMatrix((float *)&modelMatrix);

        it->second->setWorldTransform(transform);
        m_collisionWorld->updateSingleAabb(it->second);
    }
    else
    {
        // std::cout << "CullingManager::updateObject: object not found" << std::endl;
    }
}

btCollisionObject *CullingManager::createCollisionObject(void *userPointer, btCollisionShape *shape, const glm::mat4 &modelMatrix)
{
    btCollisionObject *object = new btCollisionObject();
    object->setUserPointer(userPointer);
    object->setCollisionShape(shape);

    btTransform transform;
    transform.setFromOpenGLMatrix((float *)&modelMatrix);
    object->setWorldTransform(transform);

    m_collisionWorld->addCollisionObject(object);
    m_collisionObjects[userPointer] = object;

    return object;
}

void CullingManager::setupFrame(glm::mat4 viewProjection)
{
    calculatePlanes(viewProjection);
}

std::vector<SelectedObject> CullingManager::getObjects(glm::vec3 from, glm::vec3 to)
{
    std::vector<SelectedObject> selectedObjects;

    btVector3 rayFrom = BulletGLM::getBulletVec3(from);
    btVector3 rayTo = BulletGLM::getBulletVec3(to);

    btCollisionWorld::AllHitsRayResultCallback rayCallback(rayFrom, rayTo);

    // Perform the ray cast against the collision world (dynamicsWorld or others)
    m_collisionWorld->rayTest(rayFrom, rayTo, rayCallback);

    // Loop through the ray cast results and collect the selected objects
    for (int i = 0; i < rayCallback.m_hitFractions.size(); i++)
    {
        const btCollisionObject *object = rayCallback.m_collisionObjects[i];

        btTransform t = object->getWorldTransform();
        btVector3 aabbMin, aabbMax;
        object->getCollisionShape()->getAabb(t, aabbMin, aabbMax);

        SelectedObject so;
        so.userPointer = object->getUserPointer();
        so.aabbMin = BulletGLM::getGLMVec3(aabbMin);
        so.aabbMax = BulletGLM::getGLMVec3(aabbMax);
        so.hitPointWorld = BulletGLM::getGLMVec3(rayCallback.m_hitPointWorld[i]);
        selectedObjects.push_back(so);
    }

    // Sort selected objects by squared distance
    std::sort(selectedObjects.begin(), selectedObjects.end(), [&from](const SelectedObject &a, const SelectedObject &b)
              { return glm::length(a.hitPointWorld - from) < glm::length(b.hitPointWorld - from); });

    return selectedObjects;
}

std::vector<SelectedObject> CullingManager::getObjects(glm::vec3 aabbMin, glm::vec3 aabbMax, glm::vec3 viewPos)
{
    // auto start = std::chrono::high_resolution_clock::now();

    MyOverlapCallback *aabbCallback = new MyOverlapCallback();
    btBroadphaseAabbCallback *callback = aabbCallback;

    m_broadphase->aabbTest(BulletGLM::getBulletVec3(aabbMin), BulletGLM::getBulletVec3(aabbMax), *callback);

    // view frustum culling
    std::vector<SelectedObject> visibleObjects;
    for (int i = 0; i < aabbCallback->m_overlappingObjects->size(); i++)
    {
        btCollisionObject *object = (*aabbCallback->m_overlappingObjects)[i];

        btTransform t = object->getWorldTransform();
        btVector3 aabbMin, aabbMax;
        object->getCollisionShape()->getAabb(t, aabbMin, aabbMax);

        if (inFrustum(BulletGLM::getGLMVec3(aabbMin), BulletGLM::getGLMVec3(aabbMax), viewPos))
        {
            SelectedObject so;
            so.userPointer = object->getUserPointer();
            so.aabbMin = BulletGLM::getGLMVec3(aabbMin);
            so.aabbMax = BulletGLM::getGLMVec3(aabbMax);
            // TODO: ?
            so.hitPointWorld = glm::vec3(0.f);
            visibleObjects.push_back(so);
        }
    }

    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "CullingManager::getObjects: elapsed time: " << duration.count() << " microseconds" << std::endl;

    delete aabbCallback;

    return visibleObjects;
}

bool CullingManager::inFrustum(glm::vec3 aabbMin, glm::vec3 aabbMax, glm::vec3 viewPos)
{
    for (int i = 0; i < 6; ++i)
    {
        glm::vec3 normal = glm::vec3(m_planes[i][0], m_planes[i][1], m_planes[i][2]);
        float distance = m_planes[i][3];

        // Calculate the distance from the AABB center to the plane
        glm::vec3 aabbCenter = (aabbMin + aabbMax) * 0.5f;
        float d = glm::dot(normal, aabbCenter) + distance;

        // Calculate the maximum extent of the AABB along the plane normal
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        float projectedExtent = glm::dot(glm::abs(extents), glm::abs(normal));

        // If the AABB is entirely outside the plane, cull it
        if (d < -projectedExtent)
        {
            return false;
        }
    }

    // The AABB is either fully or partially inside the frustum
    return true;
}

void CullingManager::calculatePlanes(glm::mat4 viewProjection)
{
    glm::mat4 mat = viewProjection;

    for (int i = 4; i--;)
        m_planes[0][i] = mat[i][3] + mat[i][0]; // left
    for (int i = 4; i--;)
        m_planes[1][i] = mat[i][3] - mat[i][0]; // right
    for (int i = 4; i--;)
        m_planes[2][i] = mat[i][3] + mat[i][1]; // bottom
    for (int i = 4; i--;)
        m_planes[3][i] = mat[i][3] - mat[i][1]; // top
    for (int i = 4; i--;)
        m_planes[4][i] = mat[i][3] + mat[i][2]; // near
    for (int i = 4; i--;)
        m_planes[5][i] = mat[i][3] - mat[i][2]; // far
    for (int i = 6; i--;)
        m_planes[i] = glm::normalize(m_planes[i]);
}
