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
    static void renderHinge(int index, btHingeConstraint *constraint)
    {
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Hinge");

        ImGui::TableSetColumnIndex(2);
        bool isMotorEnabled = (constraint->getMaxMotorImpulse() > 0.0);
        ImGui::Text(isMotorEnabled ? "enabled" : "disabled");

        ImGui::TableSetColumnIndex(3);
        glm::vec2 limit(0.f);
        limit.x = constraint->getLowerLimit();
        limit.y = constraint->getUpperLimit();
        glm::vec2 copy = limit;
        VectorUI::renderVec2((std::to_string(index) + ":limit").c_str(), limit, 0.001f);
        if (limit != copy)
            constraint->setLimit(limit.x, limit.y);

        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%.3f", (float)constraint->getHingeAngle());

        ImGui::TableSetColumnIndex(5);
        btVector3 &originA = constraint->getAFrame().getOrigin();
        VectorUI::renderVec3((std::to_string(index) + ":aFrame").c_str(), originA, 0.001f);

        btVector3 &originB = constraint->getBFrame().getOrigin();
        VectorUI::renderVec3((std::to_string(index) + ":bFrame").c_str(), originB, 0.001f);
    }
};

#endif /* physics_ui_hpp */
