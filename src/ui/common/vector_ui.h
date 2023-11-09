#ifndef vector_ui_hpp
#define vector_ui_hpp

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "btBulletDynamicsCommon.h"

#include "../../external/imgui/imgui.h"
#include "../../transform/transform.h"
#include "../../utils/bullet_glm.h"

class VectorUI
{
public:
    static bool renderVec2(const char *header, glm::vec2 &vec, float dragSpeed);

    static bool renderVec3(const char *header, glm::vec3 &vec, float dragSpeed);
    static bool renderVec3(const char *header, btVector3 &vec, float dragSpeed);

    static bool renderVec4(const char *header, glm::vec4 &vec, float dragSpeed);
    static bool renderQuat(const char *header, glm::quat &quat, float dragSpeed);
    static bool renderQuatEuler(const char *header, btQuaternion &quat, float dragSpeed);
    static bool renderQuat(const char *header, btQuaternion &quat, float dragSpeed);
    static bool renderQuatEuler(const char *header, glm::quat &quat, float dragSpeed);
    static bool renderNormalizedQuat(const char *header, glm::quat &quat, float dragSpeed);
    static bool renderNormalizedQuat(const char *header, glm::vec4 &vector, float dragSpeed);

    static bool renderTransform(const char *header, eTransform &transform, float dragPos, float dragRot, float dragScale);
};

#endif /* vector_ui_hpp */
