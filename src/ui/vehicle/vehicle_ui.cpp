#include "vehicle_ui.h"

void VehicleUI::render()
{
    if (!ImGui::CollapsingHeader("Vehicle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

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
    if (ImGui::DragFloat("wheel damping", &m_vehicle->wheelDamping, 0.1f))
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
    float mass = m_vehicle->m_carChassis->getMass();
    if (ImGui::DragFloat("mass", &mass, 1.0f, 1, 10000))
    {
        btVector3 interia;
        m_vehicle->m_carChassis->getCollisionShape()->calculateLocalInertia(mass, interia);
        m_vehicle->m_carChassis->setMassProps(mass, interia);
    }
}
