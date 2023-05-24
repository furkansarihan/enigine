#include "system_monitor_ui.h"

void SystemMonitorUI::render()
{
    if (!ImGui::CollapsingHeader("System Monitor", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::IsMousePosValid())
        ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
    else
        ImGui::Text("Mouse Position: <invalid>");
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("RAM: %d", (int)m_info->resident_size);
}
