#ifndef physics_ui_hpp
#define physics_ui_hpp

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "btBulletDynamicsCommon.h"

#include "vector_ui.h"
#include "../../external/imgui/imgui.h"

class PhysicsUI
{
public:
    static void renderHinge(int index, btHingeConstraint *constraint);
    static void renderConeTwist(int index, btConeTwistConstraint *constraint);
};

#endif /* physics_ui_hpp */
