#include "particle_ui.h"

void ParticleUI::render()
{
    if (!ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    for (int i = 0; i < m_particleEngines.size(); i++)
        renderParticleEngine(m_particleEngines[i], i);
}

void ParticleUI::renderParticleEngine(ParticleEngine *pe, int index)
{
    ImGui::Text("size: %d", (int)pe->m_particles.size());
    ImGui::DragFloat((std::to_string(index) + ":m_particlesPerSecond").c_str(), &pe->m_particlesPerSecond, 1.f);
    ImGui::DragFloat((std::to_string(index) + ":m_randomness").c_str(), &pe->m_randomness, 0.01f);
    ImGui::DragFloat((std::to_string(index) + ":m_particleScale").c_str(), &pe->m_particleScale, 0.01f);
    ImGui::DragFloat((std::to_string(index) + ":m_minVelocity").c_str(), &pe->m_minVelocity, 0.01f);
    ImGui::DragFloat((std::to_string(index) + ":m_maxVelocity").c_str(), &pe->m_maxVelocity, 0.01f);
    ImGui::DragFloat((std::to_string(index) + ":m_minDuration").c_str(), &pe->m_minDuration, 0.01f);
    ImGui::DragFloat((std::to_string(index) + ":m_maxDuration").c_str(), &pe->m_maxDuration, 0.01f);
    if (ImGui::CollapsingHeader((std::to_string(index) + ":m_direction").c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat((std::to_string(index) + ":m_m_directionX").c_str(), &pe->m_direction.x, 0.01f);
        ImGui::DragFloat((std::to_string(index) + ":m_m_directionY").c_str(), &pe->m_direction.y, 0.01f);
        ImGui::DragFloat((std::to_string(index) + ":m_m_directionZ").c_str(), &pe->m_direction.z, 0.01f);
    }
    ImGui::Separator();
}