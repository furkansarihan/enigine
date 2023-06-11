#include "vehicle_ui.h"

VehicleUI::VehicleUI(Vehicle *vehicle) : m_vehicle(vehicle)
{
    m_doorOffsets[0] = glm::vec3(1.51f, 1.95f, 0.62);
    m_doorOffsets[1] = glm::vec3(-1.46f, 1.95f, 0.62);
    m_doorOffsets[2] = glm::vec3(1.46f, 2.01f, -1.33);
    m_doorOffsets[3] = glm::vec3(-1.46f, 2.01f, -1.32);

    m_pos = BulletGLM::getGLMVec3(m_vehicle->m_carChassis->getWorldTransform().getOrigin());
    m_posTarget = m_pos;
}

void VehicleUI::render()
{
    if (!ImGui::CollapsingHeader("Vehicle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("m_controlVehicle", &m_controlVehicle);
    ImGui::Checkbox("m_followVehicle", &m_followVehicle);
    renderVec3("m_followOffset", m_followOffset, 0.1f);
    ImGui::DragFloat("m_followDistance", &m_followDistance, 0.1f);
    ImGui::DragFloat("m_posFactor", &m_posFactor, 0.1f);
    ImGui::DragFloat("m_scale", &m_scale, 0.001f);
    renderVec3("m_bodyOffset", m_bodyOffset, 0.01f);
    renderVec3("m_wheelOffset", m_wheelOffset, 0.01f);
    renderVec3("m_rotation", m_rotation, 0.001f);
    renderVec3("m_bodyRotation", m_bodyRotation, 0.001f);
    renderVec3("m_hoodOffset", m_hoodOffset, 0.01f);
    renderVec3("m_trunkOffset", m_trunkOffset, 0.01f);
    for (int i = 0; i < 4; i++)
        renderVec3((std::string("m_doorOffsets:") + std::to_string(i)).c_str(), m_doorOffsets[i], 0.01f);
    ImGui::Text("gVehicleSteering = %f", m_vehicle->gVehicleSteering);
    ImGui::DragFloat("steeringClamp", &m_vehicle->steeringClamp, 0.1f);
    ImGui::DragFloat("maxEngineForce", &m_vehicle->maxEngineForce, 2.0f);
    ImGui::DragFloat("accelerationVelocity", &m_vehicle->accelerationVelocity, 2.0f);
    ImGui::DragFloat("decreaseVelocity", &m_vehicle->decreaseVelocity, 2.0f);
    ImGui::DragFloat("breakingVelocity", &m_vehicle->breakingVelocity, 2.0f);
    ImGui::DragFloat("steeringIncrement", &m_vehicle->steeringIncrement, 0.2f);
    ImGui::DragFloat("steeringVelocity", &m_vehicle->steeringVelocity, 10.0f);
    if (ImGui::DragFloat("lowerLimit", &m_vehicle->lowerLimit, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheels[i]->setLimit(2, m_vehicle->lowerLimit, m_vehicle->upperLimit);
        }
    }
    if (ImGui::DragFloat("upperLimit", &m_vehicle->upperLimit, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheels[i]->setLimit(2, m_vehicle->lowerLimit, m_vehicle->upperLimit);
        }
    }
    if (ImGui::DragFloat("wheel damping", &m_vehicle->damping, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheelBodies[i]->setDamping(m_vehicle->damping, m_vehicle->damping);
        }
    }
    if (ImGui::DragFloat("wheel friction", &m_vehicle->friction, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheelBodies[i]->setFriction(m_vehicle->friction);
        }
    }
    if (ImGui::DragFloat("wheel stifness", &m_vehicle->stifness, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheels[i]->setStiffness(2, m_vehicle->stifness);
        }
    }
    if (ImGui::DragFloat("wheel constraint damping", &m_vehicle->wheelDamping, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheels[i]->setDamping(2, m_vehicle->wheelDamping);
        }
    }
    if (ImGui::DragFloat("wheel bounce", &m_vehicle->bounce, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheels[i]->setBounce(2, m_vehicle->bounce);
        }
    }
    float restitution = m_vehicle->wheelBodies[0]->getRestitution();
    if (ImGui::DragFloat("wheel restitution", &restitution, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            m_vehicle->wheelBodies[i]->setRestitution(restitution);
        }
    }
    float wheelMass = m_vehicle->wheelBodies[0]->getMass();
    if (ImGui::DragFloat("wheel mass", &wheelMass, 0.1f))
    {
        for (int i = 0; i < 4; i++)
        {
            btVector3 interia;
            m_vehicle->wheelBodies[i]->getCollisionShape()->calculateLocalInertia(wheelMass, interia);
            m_vehicle->wheelBodies[i]->setMassProps(wheelMass, interia);
        }
    }
    float wheelCFM = m_vehicle->wheels[0]->getParam(BT_CONSTRAINT_CFM, 2);
    if (ImGui::DragFloat("wheel BT_CONSTRAINT_CFM", &wheelCFM, 0.1f))
    {
        for (int i = 0; i < 4; i++)
            m_vehicle->wheels[i]->setParam(wheelCFM, BT_CONSTRAINT_ERP, 2);
    }
    float wheelERP = m_vehicle->wheels[0]->getParam(BT_CONSTRAINT_ERP, 2);
    if (ImGui::DragFloat("wheel BT_CONSTRAINT_ERP", &wheelERP, 0.1f))
    {
        for (int i = 0; i < 4; i++)
            m_vehicle->wheels[i]->setParam(wheelCFM, BT_CONSTRAINT_CFM, 2);
    }
    float mass = m_vehicle->m_carChassis->getMass();
    if (ImGui::DragFloat("mass", &mass, 1.0f, 1, 10000))
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
