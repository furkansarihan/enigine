#include "vehicle_ui.h"

VehicleUI::VehicleUI(Vehicle *vehicle) : m_vehicle(vehicle)
{
    m_doorOffsets[0] = glm::vec3(1.51f, 1.95f, 0.62);
    m_doorOffsets[1] = glm::vec3(-1.46f, 1.95f, 0.62);
    m_doorOffsets[2] = glm::vec3(1.46f, 2.01f, -1.33);
    m_doorOffsets[3] = glm::vec3(-1.46f, 2.01f, -1.32);
}

void VehicleUI::render()
{
    if (!ImGui::CollapsingHeader("Vehicle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("m_controlVehicle", &m_controlVehicle);
    ImGui::Checkbox("m_followVehicle", &m_followVehicle);
    renderVec3("m_followOffset", m_follow.offset, 0.1f);
    ImGui::DragFloat("m_follow.distance", &m_follow.distance, 0.1f);
    ImGui::DragFloat("m_follow.gapFactor", &m_follow.gapFactor, 0.001f);
    ImGui::DragFloat("m_follow.gap", &m_follow.gap, 0.001f);
    ImGui::DragFloat("m_follow.gapTarget", &m_follow.gapTarget, 0.001f);
    ImGui::DragFloat("m_follow.steeringFactor", &m_follow.steeringFactor, 0.001f);
    ImGui::DragFloat("m_follow.gapSpeed", &m_follow.gapSpeed, 0.001f);
    ImGui::DragFloat("m_follow.angleFactor", &m_follow.angleFactor, 0.01f);
    ImGui::DragFloat("m_follow.angleSpeed", &m_follow.angleSpeed, 0.001f);
    ImGui::DragFloat("m_scale", &m_scale, 0.001f);
    ImGui::DragFloat("m_wheelScale", &m_wheelScale, 0.001f);
    renderVec3("m_bodyOffset", m_bodyOffset, 0.01f);
    renderVec3("m_wheelOffset", m_wheelOffset, 0.01f);
    renderVec3("m_rotation", m_rotation, 0.001f);
    renderVec3("m_bodyRotation", m_bodyRotation, 0.001f);
    renderVec3("m_hoodOffset", m_hoodOffset, 0.01f);
    renderVec3("m_trunkOffset", m_trunkOffset, 0.01f);
    for (int i = 0; i < 4; i++)
        renderVec3((std::string("m_doorOffsets:") + std::to_string(i)).c_str(), m_doorOffsets[i], 0.01f);
    ImGui::Text("gVehicleSteering = %f", m_vehicle->gVehicleSteering);
    ImGui::Text("m_speed = %f", m_vehicle->m_speed);
    ImGui::Text("gEngineForce = %f", m_vehicle->gEngineForce);
    if (ImGui::DragFloat("m_wheelFriction", &m_vehicle->m_wheelFriction, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionStiffness", &m_vehicle->m_suspensionStiffness, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionDamping", &m_vehicle->m_suspensionDamping, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_suspensionCompression", &m_vehicle->m_suspensionCompression, 0.1f))
        updateWheelInfo();
    if (ImGui::DragFloat("m_rollInfluence", &m_vehicle->m_rollInfluence, 0.1f))
        updateWheelInfo();
    ImGui::Separator();
    ImGui::DragFloat("steeringClamp", &m_vehicle->steeringClamp, 0.1f);
    ImGui::DragFloat("maxEngineForce", &m_vehicle->maxEngineForce, 2.0f);
    ImGui::DragFloat("accelerationVelocity", &m_vehicle->accelerationVelocity, 2.0f);
    ImGui::DragFloat("decreaseVelocity", &m_vehicle->decreaseVelocity, 2.0f);
    ImGui::DragFloat("breakingVelocity", &m_vehicle->breakingVelocity, 2.0f);
    ImGui::DragFloat("steeringIncrement", &m_vehicle->steeringIncrement, 0.2f);
    ImGui::DragFloat("steeringSpeed", &m_vehicle->steeringSpeed, 0.001f);
    float mass = m_vehicle->m_carChassis->getMass();
    if (ImGui::DragFloat("m_carChassis.mass", &mass, 1.0f, 1, 10000))
    {
        btVector3 interia;
        m_vehicle->m_carChassis->getCollisionShape()->calculateLocalInertia(mass, interia);
        m_vehicle->m_carChassis->setMassProps(mass, interia);
    }
}

void VehicleUI::renderVec3(const char *header, glm::vec3 &vec, float dragSpeed)
{
    if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &vec.z, dragSpeed);
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
    }
}
