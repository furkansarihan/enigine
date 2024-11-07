#include "camera_ui.h"

void CameraUI::render()
{
    ImGui::PushID(this);

    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::PopID();
        return;
    }

    VectorUI::renderVec3("position##camera", m_camera->position, 0.1f);
    if (VectorUI::renderVec3("front##camera", m_camera->front, 0.001f))
        m_camera->front = glm::normalize(m_camera->front);
    ImGui::DragFloat("near", &m_camera->m_near, 0.001f);
    ImGui::DragFloat("far", &m_camera->m_far, 10.0f);
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

    ImGui::PopID();
}
