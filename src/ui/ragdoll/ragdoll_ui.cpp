#include "ragdoll_ui.h"

const char *stateNames[] = {"Loose", "Fetal"};

// TODO: malloc error?
void RagdollUI::render()
{
    // TODO: UpdateManager?
    update();

    if (!ImGui::CollapsingHeader("Ragdoll", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    renderRagdollControl();
    renderOffsets();
    renderJointTargetsTable();
    renderRagdollSize();
    renderRagdollTable();
}

void RagdollUI::update()
{
    if (m_floatObject)
    {
        btRigidBody *rb = m_ragdoll->m_bodies[m_floatIndex];
        btVector3 origin = rb->getWorldTransform().getOrigin();

        if (m_activateObject)
            rb->setActivationState(1);
        origin.setY(m_floatHeight);
        rb->getWorldTransform().setOrigin(origin);
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
}

void RagdollUI::renderRagdollControl()
{
    if (!ImGui::TreeNode("State Control##RagdollUI::renderRagdollControl"))
        return;

    btRigidBody *rb = m_ragdoll->m_bodies[m_floatIndex];
    btVector3 origin = rb->getWorldTransform().getOrigin();
    if (VectorUI::renderVec3("origin##RagdollUI::render", origin, 0.1f))
    {
        rb->getWorldTransform().setOrigin(origin);
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::Button("Activate Ragdoll"))
    {
        m_character->activateRagdoll();
    }
    ImGui::Checkbox("floatObject", &m_floatObject);
    ImGui::DragInt("floatIndex", &m_floatIndex, 1, 0, BODYPART_COUNT - 1);
    ImGui::DragFloat("m_floatHeight", &m_floatHeight, 0.1f);
    if (ImGui::BeginCombo("##RagdollStateCombo", stateNames[m_ragdoll->m_status.state]))
    {
        for (int i = 0; i < sizeof(stateNames) / sizeof(stateNames[0]); i++)
        {
            bool isSelected = (m_ragdoll->m_status.state == static_cast<RagdollState>(i));
            if (ImGui::Selectable(stateNames[i], isSelected))
            {
                m_ragdoll->changeState(static_cast<RagdollState>(i));
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Checkbox("activateObject", &m_activateObject);
    ImGui::DragFloat("stateChangeSpeed", &m_character->m_stateChangeSpeed, 0.1f);
    ImGui::DragFloat("impulseStrength", &m_character->m_impulseStrength, 0.1f);

    ImGui::TreePop();
}

void RagdollUI::renderOffsets()
{
    if (!ImGui::TreeNode("Offsets##RagdollUI::renderOffsets"))
        return;

    VectorUI::renderQuat("m_pelvisOffset", m_ragdoll->m_pelvisOffset, 0.01f);
    VectorUI::renderQuat("m_spineOffset", m_ragdoll->m_spineOffset, 0.01f);
    VectorUI::renderQuat("m_headOffset", m_ragdoll->m_headOffset, 0.01f);
    VectorUI::renderQuat("m_leftLegOffset", m_ragdoll->m_leftLegOffset, 0.01f);
    VectorUI::renderQuat("m_rightLegOffset", m_ragdoll->m_rightLegOffset, 0.01f);
    VectorUI::renderQuat("m_leftArmOffset", m_ragdoll->m_leftArmOffset, 0.01f);
    VectorUI::renderQuat("m_leftForeArmOffset", m_ragdoll->m_leftForeArmOffset, 0.01f);
    VectorUI::renderQuat("m_rightArmOffset", m_ragdoll->m_rightArmOffset, 0.01f);
    VectorUI::renderQuat("m_rightForeArmOffset", m_ragdoll->m_rightForeArmOffset, 0.01f);
    ImGui::Separator();
    VectorUI::renderVec3("m_armatureScale", m_ragdoll->m_armatureScale, 0.01f);
    VectorUI::renderQuat("m_legOffset", m_ragdoll->m_legOffset, 0.01f);
    VectorUI::renderVec3("m_modelOffset", m_ragdoll->m_modelOffset, 0.01f);

    ImGui::TreePop();
}

void RagdollUI::renderJointTargetsTable()
{
    if (!ImGui::TreeNode("Joint Targets##RagdollUI::renderJointTargetsTable"))
        return;

    ImGui::BeginTable("##JointTargetsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_WidthFixed, 300.0f);
    ImGui::TableSetupColumn("Force", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupColumn("Active");
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        JointTarget &target = m_ragdoll->m_fetalTargets[i];

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", getJointName(i).c_str());

        ImGui::TableNextColumn();
        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(m_ragdoll->m_joints[i]))
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

    ImGui::TreePop();
}

void RagdollUI::renderRagdollSize()
{
    if (!ImGui::TreeNode("Bone Sizes##RagdollUI::renderRagdollSize"))
        return;

    RagdollSize &size = m_ragdoll->m_size;
    RagdollSize copy = size;
    ImGui::DragFloat("pelvisHeight", &size.pelvisHeight, 0.01f);
    ImGui::DragFloat("spineHeight", &size.spineHeight, 0.01f);
    ImGui::DragFloat("headHeight", &size.headHeight, 0.01f);
    VectorUI::renderVec3("shoulderOffset", size.shoulderOffset, 0.001f);
    ImGui::DragFloat("lowerArmLength", &size.lowerArmLength, 0.01f);
    ImGui::DragFloat("upperArmLength", &size.upperArmLength, 0.01f);
    ImGui::DragFloat("lowerLegLength", &size.lowerLegLength, 0.01f);
    ImGui::DragFloat("upperLegLength", &size.upperLegLength, 0.01f);
    if (m_ragdoll->m_size != copy)
    {
        m_ragdoll->updateJointSizes();
        m_ragdoll->updateJointFrames();
    }

    ImGui::TreePop();
}

void RagdollUI::renderRagdollTable()
{
    if (!ImGui::TreeNode("Joints##RagdollUI::renderRagdollTable"))
        return;

    ImGui::BeginTable("##ragdollTable", 8, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Motor");
    ImGui::TableSetupColumn("Limit", ImGuiTableColumnFlags_WidthFixed, 180.0f);
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_WidthFixed, 40.0f);
    ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_WidthFixed, 300.0f);
    ImGui::TableSetupColumn("Softness");
    ImGui::TableSetupColumn("Relaxation");
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", getJointName(i).c_str());

        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(m_ragdoll->m_joints[i]))
            PhysicsUI::renderConeTwist(i, (btConeTwistConstraint *)m_ragdoll->m_joints[i]);
        else
            PhysicsUI::renderHinge(i, (btHingeConstraint *)m_ragdoll->m_joints[i]);
    }
    ImGui::EndTable();

    ImGui::TreePop();
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
