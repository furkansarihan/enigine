#include "terrain_ui.h"

void TerrainUI::render()
{
    if (!ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("wirewrame", &m_terrain->wireframe);
    ImGui::DragInt("level", &m_terrain->level);
    if (ImGui::DragFloat("scaleHoriz", &m_terrain->m_scaleHoriz, 0.05f))
    {
        m_terrain->updateHorizontalScale();
    }
    if (ImGui::DragFloat("minHeight", &m_terrain->m_minHeight, 1.0f))
    {
        m_terrain->updateHorizontalScale();
    }
    if (ImGui::DragFloat("maxHeight", &m_terrain->m_maxHeight, 1.0f))
    {
        m_terrain->updateHorizontalScale();
    }
    ImGui::DragFloat("fogMaxDist", &m_terrain->fogMaxDist, 100.0f);
    ImGui::DragFloat("fogMinDist", &m_terrain->fogMinDist, 100.0f);
    ImGui::ColorEdit4("fogColor", &m_terrain->fogColor[0]);
    ImGui::DragFloat("ambientMult", &m_terrain->ambientMult, 0.01f);
    ImGui::DragFloat("diffuseMult", &m_terrain->diffuseMult, 0.01f);
    ImGui::DragFloat("specularMult", &m_terrain->specularMult, 0.01f);
    ImGui::DragFloat("terrainCenter-X", &m_terrain->terrainCenter.x, 1.0f);
    ImGui::DragFloat("terrainCenter-Z", &m_terrain->terrainCenter.z, 1.0);
    ImGui::DragFloat("uvOffset-X", &m_terrain->uvOffset.x, 0.001f);
    ImGui::DragFloat("uvOffset-Y", &m_terrain->uvOffset.y, 0.001);
    ImGui::DragInt("grassTileSize", &m_terrain->m_grassTileSize, 1, 0, 128);
    ImGui::DragFloat("grassDensity", &m_terrain->m_grassDensity, 0.01, 0, 10);
    ImGui::DragInt("stoneTileSize", &m_terrain->m_stoneTileSize, 1, 0, 128);
    ImGui::DragFloat("stoneDensity", &m_terrain->m_stoneDensity, 0.01, 0, 10);
    ImGui::DragFloat("windIntensity", &m_terrain->m_windIntensity, 0.2, 0, 50);
    float trestitution = m_terrain->terrainBody->getRestitution();
    if (ImGui::DragFloat("terrain restitution", &trestitution, 0.1f))
    {
        m_terrain->terrainBody->setRestitution(trestitution);
    }
    ImGui::Checkbox("showCascade", &m_terrain->showCascade);
    ImGui::DragFloat("terrainBias0", &m_terrain->shadowBias.x, 0.001f);
    ImGui::DragFloat("terrainBias1", &m_terrain->shadowBias.y, 0.001f);
    ImGui::DragFloat("terrainBias2", &m_terrain->shadowBias.z, 0.001f);

    // ImGui::Text("left plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[0].x, terrain->m_planes[0].y, terrain->m_planes[0].z, terrain->m_planes[0].w);
    // ImGui::Text("right plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[1].x, terrain->m_planes[1].y, terrain->m_planes[1].z, terrain->m_planes[1].w);
}
