#include "culling_manager.h"

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
    this->init();
}

CullingManager::~CullingManager()
{
    for (int i = collisionWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject *obj = collisionWorld->getCollisionObjectArray()[i];

        // TODO: refactor when shape reuse impl
        if (obj && obj->getCollisionShape())
        {
            delete obj->getCollisionShape();
        }

        collisionWorld->removeCollisionObject(obj);

        delete obj;
    }

    delete dispatcher;
    delete broadphase;
    delete collisionConfiguration;
    delete collisionWorld;
}

void CullingManager::init()
{
    this->collisionConfiguration = new btDefaultCollisionConfiguration();
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);
    this->broadphase = new btDbvtBroadphase();
    this->collisionWorld = new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);
}

// TODO: reuse shapes
btCollisionObject *CullingManager::addObject(void *userPointer, const btScalar radius, const btVector3 &position)
{
    btCollisionShape *shape = new btSphereShape(radius);
    return this->createCollisionObject(userPointer, shape, position);
}

void CullingManager::updateObject(void *userPointer, const btVector3 &position)
{
    auto it = collisionObjects.find(userPointer);
    if (it != collisionObjects.end())
    {
        std::cout << "found id: " << it->first << std::endl;
        std::cout << "found object: " << it->second << std::endl;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(position);

        it->second->setWorldTransform(transform);
        collisionWorld->updateSingleAabb(it->second);
    }
    else
    {
        std::cout << "provided id is not found" << std::endl;
    }
}

btCollisionObject *CullingManager::createCollisionObject(void *userPointer, btCollisionShape *shape, const btVector3 &position)
{
    btCollisionObject *object = new btCollisionObject();
    object->setUserPointer(userPointer);
    object->setCollisionShape(shape);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);
    object->setWorldTransform(transform);

    collisionWorld->addCollisionObject(object);
    collisionObjects[userPointer] = object;

    return object;
}

std::vector<void *> CullingManager::getObjects(btVector3 aabbMin, btVector3 aabbMax)
{
    MyOverlapCallback *aabbCallback = new MyOverlapCallback();
    btBroadphaseAabbCallback *callback = aabbCallback;

    broadphase->aabbTest(aabbMin, aabbMax, *callback);

    std::cout << "m_overlappingObjects: size: " << aabbCallback->m_overlappingObjects->size() << std::endl;

    std::vector<void *> objects;
    for (int i = 0; i < aabbCallback->m_overlappingObjects->size(); i++)
    {
        btCollisionObject *object = (*aabbCallback->m_overlappingObjects)[i];
        std::cout << "found object: (" << object->getWorldTransform().getOrigin().getX()
                  << ", " << object->getWorldTransform().getOrigin().getY()
                  << ", " << object->getWorldTransform().getOrigin().getZ() << ")" << std::endl;
        objects.push_back(object->getUserPointer());
    }

    delete aabbCallback;

    return objects;
}
