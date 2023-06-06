#include "particle_ui.h"

void ParticleUI::render()
{
    if (!ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Text("size: %d", m_particleEngine->m_particles.size());
    ImGui::DragFloat("m_particlesPerSecond", &m_particleEngine->m_particlesPerSecond, 1.f);
    ImGui::DragFloat("m_randomness", &m_particleEngine->m_randomness, 0.01f);
    ImGui::DragFloat("m_particleScale", &m_particleEngine->m_particleScale, 0.01f);
    ImGui::DragFloat("m_minVelocity", &m_particleEngine->m_minVelocity, 0.01f);
    ImGui::DragFloat("m_maxVelocity", &m_particleEngine->m_maxVelocity, 0.01f);
    ImGui::DragFloat("m_followDist", &m_followDist, 0.25f);
    ImGui::DragFloat("m_followOffsetY", &m_followOffsetY, 0.1f);
}
