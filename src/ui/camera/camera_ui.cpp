#include "camera_ui.h"

void CameraUI::render()
{
    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Text("position: (%.1f, %.1f, %.1f)", m_camera->position.x, m_camera->position.y, m_camera->position.z);
    ImGui::Text("front: (%.3f, %.3f, %.3f)", m_camera->front.x, m_camera->front.y, m_camera->front.z);
    ImGui::DragFloat("mouseSensitivity", &m_camera->mouseSensitivity, 0.001f);
    ImGui::DragFloat("near", &m_camera->near, 0.001f);
    ImGui::DragFloat("far", &m_camera->far, 10.0f);
    float fovDegrees = glm::degrees(m_camera->fov);
    if (ImGui::DragFloat("fov", &fovDegrees, 0.1f))
        m_camera->fov = glm::radians(fovDegrees);
    ImGui::DragFloat("movementSpeed", &m_camera->movementSpeed, 10.0f);
    ImGui::DragFloat("scaleOrtho", &m_camera->scaleOrtho, 0.1f);
    // ImGui::DragFloat("blurOffset", &blurOffset, 0.001f);
    if (ImGui::RadioButton("perspective", m_camera->projectionMode == ProjectionMode::Perspective))
    {
        m_camera->projectionMode = ProjectionMode::Perspective;
    }
    if (ImGui::RadioButton("ortho", m_camera->projectionMode == ProjectionMode::Ortho))
    {
        m_camera->projectionMode = ProjectionMode::Ortho;
    }
}
