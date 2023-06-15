#ifndef bullet_glm_hpp
#define bullet_glm_hpp

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class BulletGLM
{
public:
    static inline glm::vec3 getGLMVec3(const btVector3 &vec)
    {
        return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
    }

    static inline btVector3 getBulletVec3(const glm::vec3 &vec)
    {
        return btVector3(vec.x, vec.y, vec.z);
    }

    static inline glm::quat getGLMQuat(const btQuaternion &quat)
    {
        return glm::quat(quat.getW(), quat.getX(), quat.getY(), quat.getZ());
    }

    static inline btQuaternion getBulletQuat(const glm::quat &quat)
    {
        return btQuaternion(quat.x, quat.y, quat.z, quat.w);
    }
};

#endif /* bullet_glm_hpp */
