#include "terrain_ui.h"

void TerrainUI::render()
{
    if (!ImGui::CollapsingHeader("Terrain", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("m_drawHeightCells", &m_terrain->m_drawHeightCells);
    ImGui::Checkbox("wirewrame", &m_terrain->wireframe);
    ImGui::DragInt("level", &m_terrain->level);
    if (ImGui::DragFloat("scaleHoriz", &m_terrain->m_scaleHoriz, 0.05f))
        m_terrain->updateHorizontalScale();
    if (ImGui::DragFloat("m_heightCellSize", &m_terrain->m_heightCellSize, 1.0f, 2.f, 1024.f))
        m_terrain->updateHorizontalScale();
    ImGui::DragFloat("m_cellScaleMult", &m_terrain->m_cellScaleMult, 0.01f);
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
    VectorUI::renderVec3("m_grassColorFactor", m_terrain->m_grassColorFactor, 0.01f);
    ImGui::DragInt("stoneTileSize", &m_terrain->m_stoneTileSize, 1, 0, 128);
    ImGui::DragFloat("stoneDensity", &m_terrain->m_stoneDensity, 0.01, 0, 10);
    ImGui::DragFloat("windIntensity", &m_terrain->m_windIntensity, 0.2, 0, 50);
    float trestitution = m_terrain->terrainBody->getRestitution();
    if (ImGui::DragFloat("terrain restitution", &trestitution, 0.05f))
    {
        m_terrain->terrainBody->setRestitution(trestitution);
    }
    float contactStiffness = m_terrain->terrainBody->getContactStiffness();
    float contactDamping = m_terrain->terrainBody->getContactDamping();
    if (ImGui::DragFloat("terrain contactStiffness", &contactStiffness, 0.05f))
    {
        m_terrain->terrainBody->setContactStiffnessAndDamping(contactStiffness, contactDamping);
    }
    if (ImGui::DragFloat("terrain contactDamping", &contactDamping, 0.05f))
    {
        m_terrain->terrainBody->setContactStiffnessAndDamping(contactStiffness, contactDamping);
    }
    ImGui::Checkbox("showCascade", &m_terrain->showCascade);

    // ImGui::Text("left plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[0].x, terrain->m_planes[0].y, terrain->m_planes[0].z, terrain->m_planes[0].w);
    // ImGui::Text("right plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[1].x, terrain->m_planes[1].y, terrain->m_planes[1].z, terrain->m_planes[1].w);
}

void TerrainUI::drawHeightCells(Shader &shader, Model &cube, glm::mat4 viewProjection)
{
    if (!m_terrain->m_drawHeightCells)
        return;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (int i = 0; i < m_terrain->m_horizontalCellCount; i++)
    {
        for (int j = 0; j < m_terrain->m_verticalCellCount; j++)
        {
            HeightCell &heightCell = m_terrain->m_heightMatrix[i][j];
            bool culled = m_terrain->m_heightMatrixCulled[i][j];

            float halfCellSize = m_terrain->m_heightCellSize / 2;
            float scaleFactor = m_terrain->m_maxHeight - m_terrain->m_minHeight;
            float scaledHeight = scaleFactor * (heightCell.max - heightCell.min);
            glm::vec3 pos(i * m_terrain->m_heightCellSize + halfCellSize, heightCell.min * scaleFactor + scaledHeight / 2, j * m_terrain->m_heightCellSize + halfCellSize);
            glm::vec3 scale(m_terrain->m_heightCellSize, scaledHeight + 0.1f, m_terrain->m_heightCellSize);

            pos.x *= m_terrain->m_scaleHoriz;
            pos.z *= m_terrain->m_scaleHoriz;

            scale.x *= m_terrain->m_scaleHoriz;
            scale.z *= m_terrain->m_scaleHoriz;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, scale * 0.5f);

            shader.use();
            shader.setMat4("MVP", viewProjection * model);
            shader.setVec4("DiffuseColor", culled ? glm::vec4(1.0, 0.0, 0.0, 1.0f) : glm::vec4(0.0, 1.0, 0.0, 1.0f));
            cube.draw(shader);
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
