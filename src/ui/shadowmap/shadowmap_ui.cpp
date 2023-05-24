#include "shadowmap_ui.h"

void ShadowmapUI::render()
{
    if (!ImGui::CollapsingHeader("Shadowmap", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Checkbox("drawShadowmap", &m_drawShadowmap);
    ImGui::Checkbox("drawFrustum", &m_drawFrustum);
    ImGui::Checkbox("drawAABB", &m_drawAABB);
    ImGui::DragFloat("quadScale", &m_quadScale, 0.1f);
    ImGui::DragFloat("splitWeight", &m_shadowManager->m_splitWeight, 0.01f);
    ImGui::DragFloat("bias", &m_shadowManager->m_bias, 0.001f);
    ImGui::Separator();
    ImGui::Text("Light");
    ImGui::DragFloat("X", &m_shadowManager->m_lightPos.x, 0.01f);
    ImGui::DragFloat("Y", &m_shadowManager->m_lightPos.y, 0.01f);
    ImGui::DragFloat("Z", &m_shadowManager->m_lightPos.z, 0.01f);
    ImGui::Text("Light - look at");
    ImGui::DragFloat("llaX", &m_shadowManager->m_lightLookAt.x, 0.01f);
    ImGui::DragFloat("llaY", &m_shadowManager->m_lightLookAt.y, 0.01);
    ImGui::DragFloat("llaZ", &m_shadowManager->m_lightLookAt.z, 0.01);
    ImGui::Text("camPos");
    ImGui::DragFloat("camPosX", &m_shadowManager->m_camera->position.x, 0.5f);
    ImGui::DragFloat("camPosY", &m_shadowManager->m_camera->position.y, 0.5f);
    ImGui::DragFloat("camPosZ", &m_shadowManager->m_camera->position.z, 0.5f);
    ImGui::Text("camView");
    ImGui::DragFloat("camViewX", &m_shadowManager->m_camera->front.x, 0.01f);
    ImGui::DragFloat("camViewY", &m_shadowManager->m_camera->front.y, 0.01f);
    ImGui::DragFloat("camViewZ", &m_shadowManager->m_camera->front.z, 0.01f);
    m_shadowManager->m_camera->front = glm::normalize(m_shadowManager->m_camera->front);
    ImGui::DragFloat("camNear", &m_shadowManager->m_near, 1);
    ImGui::DragFloat("camFar", &m_shadowManager->m_far, 1, 26, 1000);
}
