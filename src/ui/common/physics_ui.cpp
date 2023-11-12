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
    if (ImGui::DragFloat(("##PhysicsUI::renderConeTwist::softness" + std::to_string(index)).c_str(), &softness, 0.1f))
        constraint->setLimit(limit.x, limit.y, softness);

    ImGui::TableSetColumnIndex(7);
    float relaxation = constraint->getLimitRelaxationFactor();
    if (ImGui::DragFloat(("##PhysicsUI::renderConeTwist::relaxation" + std::to_string(index)).c_str(), &relaxation, 0.1f))
        constraint->setLimit(limit.x, limit.y, softness, 0.3f, relaxation);
}

void PhysicsUI::renderConeTwist(int index, btConeTwistConstraint *constraint)
{
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Cone Twist");

    ImGui::TableSetColumnIndex(2);
    ImGui::Text(constraint->isMotorEnabled() ? "enabled" : "disabled");

    ImGui::TableSetColumnIndex(3);
    glm::vec3 limit(0.f);
    limit.x = constraint->getLimit(5); // swingSpan1
    limit.y = constraint->getLimit(4); // swingSpan2
    limit.z = constraint->getLimit(3); // twistSpan
    if (VectorUI::renderVec3(("##PhysicsUI::renderConeTwist::limit" + std::to_string(index)).c_str(), limit, 0.001f))
        constraint->setLimit(limit.x, limit.y, limit.z);

    ImGui::TableSetColumnIndex(4);
    ImGui::Text("%.3f", (float)constraint->getTwistAngle());

    ImGui::TableSetColumnIndex(5);
    btTransform frameA = constraint->getAFrame();
    btTransform frameB = constraint->getBFrame();
    btVector3 &originA = frameA.getOrigin();
    btVector3 &originB = frameB.getOrigin();
    btQuaternion rotationA = frameA.getRotation();
    btQuaternion rotationB = frameB.getRotation();

    if (VectorUI::renderVec3(("##PhysicsUI::renderConeTwist::aFrameOrigin" + std::to_string(index)).c_str(), originA, 0.001f))
        constraint->setFrames(frameA, frameB);
    if (VectorUI::renderVec3(("##PhysicsUI::renderConeTwist::bFrameOrigin" + std::to_string(index)).c_str(), originB, 0.001f))
        constraint->setFrames(frameA, frameB);
    if (VectorUI::renderQuat(("##PhysicsUI::renderConeTwist::aFrameRot" + std::to_string(index)).c_str(), rotationA, 0.001f))
    {
        frameA.setRotation(rotationA);
        constraint->setFrames(frameA, frameB);
    }
    if (VectorUI::renderQuat(("##PhysicsUI::renderConeTwist::bFrameRot" + std::to_string(index)).c_str(), rotationB, 0.001f))
    {
        frameB.setRotation(rotationB);
        constraint->setFrames(frameA, frameB);
    }

    ImGui::TableSetColumnIndex(6);
    float softness = constraint->getLimitSoftness();
    if (ImGui::DragFloat(("##PhysicsUI::renderConeTwist::softness" + std::to_string(index)).c_str(), &softness, 0.1f))
        constraint->setLimit(limit.x, limit.y, limit.z, softness);

    ImGui::TableSetColumnIndex(7);
    float relaxation = constraint->getRelaxationFactor();
    if (ImGui::DragFloat(("##PhysicsUI::renderConeTwist::relaxation" + std::to_string(index)).c_str(), &relaxation, 0.1f))
        constraint->setLimit(limit.x, limit.y, limit.z, softness, 0.3f, relaxation);
}
