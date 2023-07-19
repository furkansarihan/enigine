#include "temp_ui.h"

void TempUI::render()
{
    if (!ImGui::CollapsingHeader("Others", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    if (ImGui::Button("refresh shaders"))
    {
        m_shaderManager->initShaders();
    }
    ImGui::Text("Post process");
    ImGui::DragFloat("exposure", &m_postProcess->m_exposure, 0.001);
    ImGui::Separator();
    ImGui::Text("Physics");
    ImGui::Text("m_deltaTime: %.6f", m_deltaTime);
    ImGui::DragInt("m_maxSubSteps", &m_physicsWorld->m_maxSubSteps);
    bool debugEnabled = m_debugDrawer->getDebugMode();
    if (ImGui::Checkbox("debugEnabled", &debugEnabled))
    {
        m_debugDrawer->setDebugMode(debugEnabled ? btIDebugDraw::DBG_DrawWireframe |
                                                       btIDebugDraw::DBG_DrawConstraints |
                                                       btIDebugDraw::DBG_DrawConstraintLimits
                                                 : btIDebugDraw::DBG_NoDebug);
    }
    int lines = m_debugDrawer->getLines().size();
    ImGui::DragInt("lines", &lines);
    if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_sunColor", m_sunColor, 1.f);
        ImGui::DragFloat("m_sunIntensity", &m_sunIntensity, 0.1f);
    }
    if (ImGui::CollapsingHeader("Shelter", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_shelterPos", m_shelterTransform.m_position, 0.1f);
        VectorUI::renderQuatEuler("m_shelterRot", m_shelterTransform.m_rotation, 1.0f);
        VectorUI::renderVec3("m_shelterScale", m_shelterTransform.m_scale, 0.1f);
    }
    if (ImGui::CollapsingHeader("Tower", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_towerPos", m_towerTransform.m_position, 0.1f);
        VectorUI::renderQuatEuler("m_towerRot", m_towerTransform.m_rotation, 1.0f);
        VectorUI::renderVec3("m_towerScale", m_towerTransform.m_scale, 0.1f);
    }
    ImGui::Text("Light");
    ImGui::DragFloat("power", &m_lightPower, 0.1);
    ImGui::DragFloat("radius", &m_radius, 0.1);
    ImGui::DragFloat("speed", &m_speed, 0.01);
    ImGui::ColorEdit3("lightColor", m_lightColor);
    ImGui::ColorEdit3("ambientColor", m_ambientColor);
    ImGui::ColorEdit3("specularColor", m_specularColor);
    ImGui::Checkbox("m_cullFront", &m_cullFront);
}
