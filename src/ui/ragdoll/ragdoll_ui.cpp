#include "ragdoll_ui.h"

const char *stateNames[] = {"Loose", "Fetal"};

// TODO: malloc error?
void RagdollUI::render()
{
    if (!ImGui::CollapsingHeader("Ragdoll", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    Ragdoll *ragdoll = m_character->m_ragdoll;
    if (ImGui::Button("Activate Ragdoll"))
    {
        m_character->activateRagdoll();
    }
    ImGui::Checkbox("floatObject", &m_floatObject);
    if (ImGui::BeginCombo("##RagdollStateCombo", stateNames[ragdoll->m_status.state]))
    {
        for (int i = 0; i < sizeof(stateNames) / sizeof(stateNames[0]); i++)
        {
            bool isSelected = (ragdoll->m_status.state == static_cast<RagdollState>(i));
            if (ImGui::Selectable(stateNames[i], isSelected))
            {
                ragdoll->changeState(static_cast<RagdollState>(i));
            }
        }
        ImGui::EndCombo();
    }

    ImGui::BeginTable("JointTargetsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Force", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Active");
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        JointTarget &target = ragdoll->m_fetalTargets[i];

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", getJointName(i).c_str());

        ImGui::TableNextColumn();
        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(ragdoll->m_joints[i]))
            VectorUI::renderNormalizedQuat(("##JointTargetsTable::angle:" + std::to_string(i)).c_str(), target.angle, 0.01f);
        else
            // VectorUI::renderNormalizedQuat(("##JointTargetsTable::angle:" + std::to_string(i)).c_str(), target.angle, 0.01f);
            ImGui::DragFloat(("##JointTargetsTable::angle:" + std::to_string(i)).c_str(), &target.angle.x, 0.01f);

        ImGui::TableNextColumn();
        ImGui::DragFloat(("##JointTargetsTable::force:" + std::to_string(i)).c_str(), &target.force, 0.1f);

        ImGui::TableNextColumn();
        ImGui::Checkbox(("##JointTargetsTable::active:" + std::to_string(i)).c_str(), &target.active);
    }
    ImGui::EndTable();

    RagdollSize &size = ragdoll->m_size;
    RagdollSize copy = size;
    ImGui::DragFloat("pelvisHeight", &size.pelvisHeight, 0.01f);
    ImGui::DragFloat("spineHeight", &size.spineHeight, 0.01f);
    ImGui::DragFloat("headHeight", &size.headHeight, 0.01f);
    VectorUI::renderVec3("shoulderOffset", size.shoulderOffset, 0.001f);
    ImGui::DragFloat("lowerArmLength", &size.lowerArmLength, 0.01f);
    ImGui::DragFloat("upperArmLength", &size.upperArmLength, 0.01f);
    ImGui::DragFloat("lowerLegLength", &size.lowerLegLength, 0.01f);
    ImGui::DragFloat("upperLegLength", &size.upperLegLength, 0.01f);
    if (size != copy)
    {
        ragdoll->updateJointSizes();
        ragdoll->updateJointFrames();
    }
    ImGui::Separator();

    VectorUI::renderQuatEuler("m_rightArmOffset", ragdoll->m_rightArmOffset, 0.01f);
    VectorUI::renderQuatEuler("m_leftArmOffset", ragdoll->m_leftArmOffset, 0.01f);
    VectorUI::renderVec3("m_modelOffset", ragdoll->m_modelOffset, 0.01f);
    ImGui::DragFloat("m_floatHeight", &m_floatHeight, 0.1f);
    ImGui::Checkbox("activateObject", &m_activateObject);
    ImGui::DragInt("floatIndex", &m_floatIndex, 1, 0, BODYPART_COUNT - 1);
    ImGui::DragFloat("stateChangeSpeed", &m_character->m_stateChangeSpeed, 0.1f);
    ImGui::DragFloat("impulseStrength", &m_character->m_impulseStrength, 0.1f);
    btRigidBody *rb = ragdoll->m_bodies[m_floatIndex];
    btVector3 origin = rb->getWorldTransform().getOrigin();
    if (VectorUI::renderVec3("origin##RagdollUI::render", origin, 0.1f))
    {
        rb->getWorldTransform().setOrigin(origin);
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (m_floatObject)
    {
        if (m_activateObject)
            rb->setActivationState(1);
        origin.setY(m_floatHeight);
        rb->getWorldTransform().setOrigin(origin);
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }

    ImGui::BeginTable("Ragdoll", 8, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Motor");
    ImGui::TableSetupColumn("Limit", ImGuiTableColumnFlags_WidthFixed, 180.0f);
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_WidthFixed, 40.0f);
    ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_WidthFixed, 200.0f);
    ImGui::TableSetupColumn("Softness");
    ImGui::TableSetupColumn("Relaxation");
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", getJointName(i).c_str());

        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(ragdoll->m_joints[i]))
            renderConeTwist(i, (btConeTwistConstraint *)ragdoll->m_joints[i]);
        else
            PhysicsUI::renderHinge(i, (btHingeConstraint *)ragdoll->m_joints[i]);
    }
    ImGui::EndTable();
}

void RagdollUI::renderConeTwist(int index, btConeTwistConstraint *constraint)
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
    if (VectorUI::renderVec3(("##RagdollUI::renderConeTwist::limit" + std::to_string(index)).c_str(), limit, 0.001f))
        constraint->setLimit(limit.x, limit.y, limit.z);

    ImGui::TableSetColumnIndex(4);
    ImGui::Text("%.3f", (float)constraint->getTwistAngle());

    ImGui::TableSetColumnIndex(5);
    btTransform frameA = constraint->getAFrame();
    btTransform frameB = constraint->getBFrame();
    btVector3 &originA = frameA.getOrigin();
    btVector3 &originB = frameB.getOrigin();

    if (VectorUI::renderVec3(("##RagdollUI::renderConeTwist::aFrame" + std::to_string(index)).c_str(), originA, 0.001f))
        constraint->setFrames(frameA, frameB);

    if (VectorUI::renderVec3(("##RagdollUI::renderConeTwist::bFrame" + std::to_string(index)).c_str(), originB, 0.001f))
        constraint->setFrames(frameA, frameB);

    ImGui::TableSetColumnIndex(6);
    float softness = constraint->getLimitSoftness();
    if (ImGui::DragFloat(("##RagdollUI::renderConeTwist::softness" + std::to_string(index)).c_str(), &softness, 0.1f))
        constraint->setLimit(limit.x, limit.y, limit.z, softness);

    ImGui::TableSetColumnIndex(7);
    float relaxation = constraint->getRelaxationFactor();
    if (ImGui::DragFloat(("##RagdollUI::renderConeTwist::relaxation" + std::to_string(index)).c_str(), &relaxation, 0.1f))
        constraint->setLimit(limit.x, limit.y, limit.z, softness, 0.3f, relaxation);
}

std::string RagdollUI::getJointName(int index)
{
    switch (index)
    {
    case JOINT_PELVIS_SPINE:
        return "PELVIS_SPINE";
    case JOINT_SPINE_HEAD:
        return "SPINE_HEAD";
    case JOINT_LEFT_HIP:
        return "LEFT_HIP";
    case JOINT_LEFT_KNEE:
        return "LEFT_KNEE";
    case JOINT_RIGHT_HIP:
        return "RIGHT_HIP";
    case JOINT_RIGHT_KNEE:
        return "RIGHT_KNEE";
    case JOINT_LEFT_SHOULDER:
        return "LEFT_SHOULDER";
    case JOINT_LEFT_ELBOW:
        return "LEFT_ELBOW";
    case JOINT_RIGHT_SHOULDER:
        return "RIGHT_SHOULDER";
    case JOINT_RIGHT_ELBOW:
        return "RIGHT_ELBOW";
    case JOINT_COUNT:
        return "COUNT";
    default:
        return "UNKNOWN_JOINT";
    }
}
