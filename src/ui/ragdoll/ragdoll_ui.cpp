#include "ragdoll_ui.h"

const char *stateNames[] = {"Loose", "Fetal"};

void renderQuat(const char *header, glm::vec4 &vector, float dragSpeed);
void renderVec3(const char *header, glm::vec3 &vector, float dragSpeed);
void renderVec2(const char *header, glm::vec2 &vector, float dragSpeed);

void RagdollUI::render()
{
    if (!ImGui::CollapsingHeader("Ragdoll", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    Ragdoll *ragdoll = m_character->m_ragdoll;
    if (ImGui::Button("Activate Ragdoll"))
    {
        m_character->activateRagdoll(glm::vec3(1.f, 0.f, 0.f));
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

    ImGui::BeginTable("JointTargetsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Force", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_None);
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        JointTarget &target = ragdoll->m_fetalTargets[i];

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", getJointName(i).c_str());

        ImGui::TableNextColumn();
        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(ragdoll->m_joints[i]))
            renderQuat((std::to_string(i) + ":angle").c_str(), target.angle, 0.01f);
        else
            ImGui::DragFloat((std::to_string(i) + ":angle").c_str(), &target.angle.x, 0.01f);

        ImGui::TableNextColumn();
        ImGui::DragFloat((std::to_string(i) + ":force").c_str(), &target.force, 0.1f);

        ImGui::TableNextColumn();
        ImGui::Checkbox((std::to_string(i) + ":active").c_str(), &target.active);
    }
    ImGui::EndTable();

    RagdollSize &size = ragdoll->m_size;
    RagdollSize copy = size;
    ImGui::DragFloat("pelvisHeight", &size.pelvisHeight, 0.01f);
    ImGui::DragFloat("spineHeight", &size.spineHeight, 0.01f);
    ImGui::DragFloat("headHeight", &size.headHeight, 0.01f);
    ImGui::DragFloat("shoulderOffsetHorizontal", &size.shoulderOffsetHorizontal, 0.01f);
    ImGui::DragFloat("shoulderOffsetVertical", &size.shoulderOffsetVertical, 0.01f);
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

    ImGui::DragFloat("m_floatHeight", &m_floatHeight, 0.1f);
    ImGui::Checkbox("activateObject", &m_activateObject);
    ImGui::DragInt("floatIndex", &m_floatIndex, 1, 0, BODYPART_COUNT - 1);
    ImGui::DragFloat("stateChangeSpeed", &m_character->m_stateChangeSpeed, 0.1f);
    ImGui::DragFloat("impulseStrength", &m_character->m_impulseStrength, 0.1f);
    btRigidBody *rb = ragdoll->m_bodies[m_floatIndex];
    float cX = rb->getWorldTransform().getOrigin().getX();
    float cY = rb->getWorldTransform().getOrigin().getY();
    float cZ = rb->getWorldTransform().getOrigin().getZ();
    if (ImGui::DragFloat("cX", &cX, 0.1f))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("cY", &cY, 0.1))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("cZ", &cZ, 0.1))
    {
        rb->getWorldTransform().setOrigin(btVector3(cX, cY, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (m_floatObject)
    {
        if (m_activateObject)
            rb->setActivationState(1);
        rb->getWorldTransform().setOrigin(btVector3(cX, m_floatHeight, cZ));
        rb->setLinearVelocity(btVector3(0, 0, 0));
    }

    ImGui::BeginTable("Ragdoll", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Motor");
    ImGui::TableSetupColumn("Limit");
    ImGui::TableSetupColumn("Angle");
    ImGui::TableHeadersRow();
    for (int i = 0; i < JOINT_COUNT; i++)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", getJointName(i).c_str());

        if (btConeTwistConstraint *h = dynamic_cast<btConeTwistConstraint *>(ragdoll->m_joints[i]))
            renderConeTwist(i, (btConeTwistConstraint *)ragdoll->m_joints[i]);
        else
            renderHinge(i, (btHingeConstraint *)ragdoll->m_joints[i]);
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
    glm::vec3 copy = limit;
    renderVec3((std::to_string(index) + ":limit").c_str(), limit, 0.001f);
    if (limit != copy)
        constraint->setLimit(limit.x, limit.y, limit.z);

    ImGui::TableSetColumnIndex(4);
    ImGui::Text("%.3f", (float)constraint->getTwistAngle());
}

void RagdollUI::renderHinge(int index, btHingeConstraint *constraint)
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
    renderVec2((std::to_string(index) + ":limit").c_str(), limit, 0.001f);
    if (limit != copy)
        constraint->setLimit(limit.x, limit.y);

    ImGui::TableSetColumnIndex(4);
    ImGui::Text("%.3f", (float)constraint->getHingeAngle());
}

void renderQuat(const char *header, glm::vec4 &vector, float dragSpeed)
{
    if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat((std::string(header) + "X").c_str(), &vector[0], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &vector[1], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &vector[2], dragSpeed);
        ImGui::DragFloat((std::string(header) + "W").c_str(), &vector[3], dragSpeed);
    }

    glm::quat normalized = glm::normalize(glm::quat(vector.w, vector.x, vector.y, vector.z));
    vector.x = normalized.x;
    vector.y = normalized.y;
    vector.z = normalized.z;
    vector.w = normalized.w;
}

void renderVec3(const char *header, glm::vec3 &vector, float dragSpeed)
{
    if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;
    ImGui::DragFloat((std::string(header) + "X").c_str(), &vector[0], dragSpeed);
    ImGui::DragFloat((std::string(header) + "Y").c_str(), &vector[1], dragSpeed);
    ImGui::DragFloat((std::string(header) + "Z").c_str(), &vector[2], dragSpeed);
}

void renderVec2(const char *header, glm::vec2 &vector, float dragSpeed)
{
    if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;
    ImGui::DragFloat((std::string(header) + "X").c_str(), &vector[0], dragSpeed);
    ImGui::DragFloat((std::string(header) + "Y").c_str(), &vector[1], dragSpeed);
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
