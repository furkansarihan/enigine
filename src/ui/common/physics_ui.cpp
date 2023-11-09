#include "physics_ui.h"

void PhysicsUI::renderHinge(int index, btHingeConstraint *constraint)
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
    VectorUI::renderVec2(("##PhysicsUI::renderHinge::limit" + std::to_string(index)).c_str(), limit, 0.001f);
    if (limit != copy)
        constraint->setLimit(limit.x, limit.y);

    ImGui::TableSetColumnIndex(4);
    ImGui::Text("%.3f", (float)constraint->getHingeAngle());

    ImGui::TableSetColumnIndex(5);
    btVector3 &originA = constraint->getAFrame().getOrigin();
    VectorUI::renderVec3(("##PhysicsUI::renderHinge::aFrame" + std::to_string(index)).c_str(), originA, 0.001f);
    btVector3 &originB = constraint->getBFrame().getOrigin();
    VectorUI::renderVec3(("##PhysicsUI::renderHinge::bFrame" + std::to_string(index)).c_str(), originB, 0.001f);

    ImGui::TableSetColumnIndex(6);
    float softness = constraint->getLimitSoftness();
    if (ImGui::DragFloat(("##RagdollUI::renderConeTwist::softness" + std::to_string(index)).c_str(), &softness, 0.1f))
        constraint->setLimit(limit.x, limit.y, softness);

    ImGui::TableSetColumnIndex(7);
    float relaxation = constraint->getLimitRelaxationFactor();
    if (ImGui::DragFloat(("##RagdollUI::renderConeTwist::relaxation" + std::to_string(index)).c_str(), &relaxation, 0.1f))
        constraint->setLimit(limit.x, limit.y, softness, 0.3f, relaxation);
}
