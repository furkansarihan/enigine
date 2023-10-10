#include "vehicle_ui.h"

const char *doorStateNames[] = {"active", "deactive"};
std::string getJointName(int index);

VehicleUI::VehicleUI(CarController *carController, Vehicle *vehicle)
    : m_cController(carController),
      m_vehicle(vehicle)
{
}

void VehicleUI::render()
{
    if (!ImGui::CollapsingHeader("Vehicle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    Follow &m_follow = m_cController->m_follow;
    ImGui::Checkbox("m_controlVehicle", &m_cController->m_controlVehicle);
    ImGui::Checkbox("m_followVehicle", &m_cController->m_followVehicle);
    VectorUI::renderVec3("m_followOffset", m_follow.offset, 0.1f);
    ImGui::DragFloat("m_follow.distance", &m_follow.distance, 0.1f);
    ImGui::DragFloat("m_follow.stretchMax", &m_follow.stretchMax, 0.001f);
    ImGui::DragFloat("m_follow.stretch", &m_follow.stretch, 0.001f);
    ImGui::DragFloat("m_follow.stretchTarget", &m_follow.stretchTarget, 0.001f);
    ImGui::DragFloat("m_follow.stretchSteeringFactor", &m_follow.stretchSteeringFactor, 0.001f);
    ImGui::DragFloat("m_follow.stretchSpeed", &m_follow.stretchSpeed, 0.001f);
    ImGui::DragFloat("m_follow.yOffset", &m_follow.yOffset, 0.01f);
    ImGui::DragFloat("m_follow.yStretchFactor", &m_follow.yStretchFactor, 0.01f);
    ImGui::DragFloat("m_follow.angularSpeed", &m_follow.angularSpeed, 0.001f);
    VectorUI::renderVec2("m_safeSize", m_cController->m_safeSize, 0.01f);
    VectorUI::renderVec2("m_doorOffset", m_cController->m_doorOffset, 0.01f);
    VectorUI::renderVec3("m_animDoorOffset", m_cController->m_animDoorOffset, 0.01f);
    renderCompoundShapeEditor("compound shapes", m_cController->m_vehicle->m_compoundShape);
    for (int i = 0; i < 4; i++)
        VectorUI::renderVec3((std::string("m_doorPosOffsets:") + std::to_string(i)).c_str(), m_cController->m_vehicle->m_doors[i].posOffset, 0.01f);
    ImGui::Text("gVehicleSteering = %f", m_vehicle->gVehicleSteering);
    ImGui::Text("m_speed = %f", m_vehicle->m_speed);
    ImGui::Text("m_vehicle.speedKmHour = %f", m_vehicle->m_vehicle->getCurrentSpeedKmHour());
    ImGui::Text("gEngineForce = %f", m_vehicle->gEngineForce);
    ImGui::Text("m_frictionSlip = %f", m_vehicle->m_vehicle->getWheelInfo(3).m_frictionSlip);
    ImGui::Text("m_speedZ = %f", m_vehicle->m_speedZ);
    ImGui::Text("m_speedRate = %f", m_vehicle->m_speedRate);
    ImGui::Text("m_speedSteerFactor = %f", m_vehicle->m_speedSteerFactor);
    glm::vec3 forwardVector = BulletGLM::getGLMVec3(m_vehicle->m_vehicle->getForwardVector());
    VectorUI::renderVec3("forwardVector", forwardVector, 0.01f);
    if (ImGui::DragFloat("m_wheelFriction", &m_vehicle->m_wheelFriction, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_driftFriction", &m_vehicle->m_driftFriction, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionStiffness", &m_vehicle->m_suspensionStiffness, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionDamping", &m_vehicle->m_suspensionDamping, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionCompression", &m_vehicle->m_suspensionCompression, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_rollInfluence", &m_vehicle->m_rollInfluence, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionRestLength", &m_vehicle->m_suspensionRestLength, 0.01f))
        updateWheelInfo();
    ImGui::Separator();
    ImGui::DragFloat("steeringLimit", &m_vehicle->steeringLimit, 0.1f);
    ImGui::DragFloat("maxEngineForce", &m_vehicle->maxEngineForce, 100.0f);
    ImGui::DragFloat("minEngineForce", &m_vehicle->minEngineForce, 100.0f);
    ImGui::DragFloat("accelerationVelocity", &m_vehicle->accelerationVelocity, 100.0f);
    ImGui::DragFloat("decreaseVelocity", &m_vehicle->decreaseVelocity, 100.0f);
    ImGui::DragFloat("breakingVelocity", &m_vehicle->breakingVelocity, 100.0f);
    ImGui::DragFloat("handBreakingVelocity", &m_vehicle->handBreakingVelocity, 100.0f);
    ImGui::DragFloat("steeringIncrement", &m_vehicle->steeringIncrement, 0.2f);
    ImGui::DragFloat("m_returnIdleFactor", &m_vehicle->m_returnIdleFactor, 0.2f);
    ImGui::DragFloat("m_maxSteerSpeed", &m_vehicle->m_maxSteerSpeed, 1.f);
    ImGui::DragFloat("m_localVelocity.x", &m_vehicle->m_localVelocity.x, 0.2f);
    ImGui::DragFloat("m_localVelocity.y", &m_vehicle->m_localVelocity.y, 0.2f);
    ImGui::DragFloat("m_localVelocity.z", &m_vehicle->m_localVelocity.z, 0.2f);
    ImGui::DragFloat("steeringSpeed", &m_vehicle->steeringSpeed, 0.001f);
    for (int i = 0; i < 4; i++)
        VectorUI::renderVec3((std::to_string(i) + "posOffsets").c_str(), m_vehicle->m_doors[i].posOffset, 0.01f);

    VectorUI::renderNormalizedQuat("m_doorRotate", m_vehicle->m_doorRotate, 0.01f);

    float mass = m_vehicle->m_carChassis->getMass();
    if (ImGui::DragFloat("m_carChassis.mass", &mass, 1.0f, 1, 10000))
    {
        btVector3 interia;
        m_vehicle->m_carChassis->getCollisionShape()->calculateLocalInertia(mass, interia);
        m_vehicle->m_carChassis->setMassProps(mass, interia);
    }

    ImGui::BeginTable("Doors", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("Motor");
    ImGui::TableSetupColumn("Limit");
    ImGui::TableSetupColumn("Angle");
    ImGui::TableSetupColumn("Frames");
    ImGui::TableSetupColumn("State");
    ImGui::TableHeadersRow();
    for (int i = 0; i < 4; i++)
    {
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", getJointName(i).c_str());
        btHingeConstraint *hinge = m_vehicle->m_doors[i].joint;
        PhysicsUI::renderHinge(i, hinge);
        ImGui::TableSetColumnIndex(6);
        renderHingeState(i);
    }
    ImGui::EndTable();

    ImGui::BeginTable("HingeTargetsTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Angle", ImGuiTableColumnFlags_None);
    ImGui::TableSetupColumn("Force", ImGuiTableColumnFlags_None);
    ImGui::TableHeadersRow();
    for (int i = 0; i < 4; i++)
    {
        HingeTarget &target = m_vehicle->m_doors[i].hingeTarget;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", getJointName(i).c_str());

        ImGui::TableNextColumn();
        ImGui::DragFloat((std::to_string(i) + ":angle").c_str(), &target.angle, 0.01f);

        ImGui::TableNextColumn();
        ImGui::DragFloat((std::to_string(i) + ":force").c_str(), &target.force, 0.1f);
    }
    ImGui::EndTable();

    for (int i = 0; i < 4; i++)
    {
        if (ImGui::Button((std::to_string(i) + ":open door").c_str()))
            m_vehicle->openDoor(i);
        ImGui::SameLine();
        if (ImGui::Button((std::to_string(i) + ":close door").c_str()))
            m_vehicle->closeDoor(i);
    }
}

std::string getJointName(int index)
{
    switch (index)
    {
    case 0:
        return "FRONT_LEFT";
    case 1:
        return "FRONT_RIGHT";
    case 2:
        return "BACK_LEFT";
    case 3:
        return "BACK_RIGHT";
    default:
        return "UNKNOWN_JOINT";
    }
}

void VehicleUI::renderCompoundShapeEditor(const char *header, btCompoundShape *compoundShape)
{
    if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    int numChildShapes = compoundShape->getNumChildShapes();
    for (int i = 0; i < numChildShapes; ++i)
    {
        btCollisionShape *childShape = compoundShape->getChildShape(i);
        btTransform &childTransform = compoundShape->getChildTransform(i);

        // Create a unique ID for ImGui widgets
        std::string childHeader = std::string(header) + " Child Shape " + std::to_string(i);

        if (!ImGui::CollapsingHeader(childHeader.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen))
            continue;

        std::string positionHeader = childHeader + ":position";
        btVector3 &childOrigin = childTransform.getOrigin();
        VectorUI::renderVec3(positionHeader.c_str(), childOrigin, 0.001f);

        std::string rotationHeader = childHeader + ":rotation";
        glm::quat childRotation = BulletGLM::getGLMQuat(childTransform.getRotation());
        glm::quat copy = childRotation;
        VectorUI::renderQuat(rotationHeader.c_str(), childRotation, 0.001f);
        if (childRotation != copy)
            childTransform.setRotation(BulletGLM::getBulletQuat(childRotation));

        // TODO:
        // Display and modify the local size of the child shape
        // btVector3 childSize;
        // if (btBoxShape *boxShape = dynamic_cast<btBoxShape *>(childShape))
        // {
        //     std::string sizeHeader = childHeader + ":size";
        //     childSize = boxShape->getHalfExtentsWithMargin();
        //     btVector3 copy = childSize;
        //     renderVec3(sizeHeader.c_str(), childSize, 0.0001f);
        //     if (childSize != copy)
        //     {
        //         m_vehicle->m_physicsWorld->m_dynamicsWorld->removeVehicle(m_vehicle->m_vehicle);
        //         boxShape->setImplicitShapeDimensions(childSize);
        //         m_vehicle->m_physicsWorld->m_dynamicsWorld->addVehicle(m_vehicle->m_vehicle);
        //     }
        // }
    }
}

void VehicleUI::renderHingeState(int index)
{
    HingeState state = m_vehicle->m_doors[index].hingeState;

    if (ImGui::BeginCombo((":" + std::to_string(index)).c_str(), doorStateNames[state]))
    {
        for (int i = 0; i < sizeof(doorStateNames) / sizeof(doorStateNames[0]); i++)
        {
            bool isSelected = (state == static_cast<HingeState>(i));
            if (ImGui::Selectable(doorStateNames[i], isSelected))
            {
                m_vehicle->updateHingeState(index, static_cast<HingeState>(i));
            }
        }
        ImGui::EndCombo();
    }
}

void VehicleUI::updateWheelInfo()
{
    for (int i = 0; i < m_vehicle->m_vehicle->getNumWheels(); i++)
    {
        btWheelInfo &wheel = m_vehicle->m_vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness = m_vehicle->m_suspensionStiffness;
        wheel.m_wheelsDampingRelaxation = m_vehicle->m_suspensionDamping;
        wheel.m_wheelsDampingCompression = m_vehicle->m_suspensionCompression;
        wheel.m_frictionSlip = m_vehicle->m_wheelFriction;
        wheel.m_rollInfluence = m_vehicle->m_rollInfluence;
        wheel.m_suspensionRestLength1 = m_vehicle->m_suspensionRestLength;
    }
}
