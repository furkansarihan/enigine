#include "camera_ui.h"

void CameraUI::render()
{
    if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Text("position: (%.1f, %.1f, %.1f)", m_camera->position.x, m_camera->position.y, m_camera->position.z);
    ImGui::Text("pitch: %.1f", m_camera->pitch);
    ImGui::Text("yaw: %.1f", m_camera->yaw);
    ImGui::DragFloat("near", &m_camera->near, 0.001f);
    ImGui::DragFloat("far", &m_camera->far, 10.0f);
    ImGui::DragFloat("fov", &m_camera->fov, 0.01f);
    ImGui::DragFloat("movementSpeed", &m_camera->movementSpeed, 10.0f);
    ImGui::DragFloat("scaleOrtho", &m_camera->scaleOrtho, 0.1f);
    // ImGui::DragFloat("blurOffset", &blurOffset, 0.001f);
    ImGui::DragFloat("followDistance", &m_camera->followDistance, 0.1f);
    ImGui::DragFloat("followOffsetY", &m_camera->followOffset.y, 0.1f);
    ImGui::Checkbox("followVehicle", &m_camera->followVehicle);
    ImGui::Checkbox("followCharacter", &m_camera->followCharacter);
    ImGui::Checkbox("controlCharacter", &m_camera->controlCharacter);
    // TODO: change camera min pitch
    if (ImGui::Checkbox("firstPerson", &m_camera->firstPerson))
    {
        if (m_camera->firstPerson)
        {
            m_camera->followDistance = -1.0f;
            m_camera->followOffset.y = 3.3f;
        }
        else
        {
            m_camera->followDistance = 10.0f;
            m_camera->followOffset.y = 1.5f;
        }
    }
    if (ImGui::RadioButton("perspective", m_camera->projectionMode == ProjectionMode::Perspective))
    {
        m_camera->projectionMode = ProjectionMode::Perspective;
    }
    if (ImGui::RadioButton("ortho", m_camera->projectionMode == ProjectionMode::Ortho))
    {
        m_camera->projectionMode = ProjectionMode::Ortho;
    }
}
