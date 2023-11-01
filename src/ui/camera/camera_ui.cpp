#include "camera_ui.h"

void CameraUI::render()
{
    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Text("position");
    ImGui::DragFloat("X##position", &m_camera->position.x, 0.5f);
    ImGui::DragFloat("Y##position", &m_camera->position.y, 0.5f);
    ImGui::DragFloat("Z##position", &m_camera->position.z, 0.5f);
    ImGui::Separator();
    ImGui::Text("front");
    ImGui::DragFloat("X##front", &m_camera->front.x, 0.01f);
    ImGui::DragFloat("Y##front", &m_camera->front.y, 0.01f);
    ImGui::DragFloat("Z##front", &m_camera->front.z, 0.01f);
    ImGui::Separator();
    ImGui::DragFloat("near", &m_camera->near, 0.001f);
    ImGui::DragFloat("far", &m_camera->far, 10.0f);
    ImGui::DragFloat("mouseSensitivity", &m_camera->mouseSensitivity, 0.001f);
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
