#include "system_monitor_ui.h"

void SystemMonitorUI::render()
{
    if (!ImGui::CollapsingHeader("System Monitor", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("RAM: %.2f MB", static_cast<float>(m_ramUsage) / (1024.0f * 1024.0f));
}

void SystemMonitorUI::update(float deltaTime)
{
    m_ramUsage = CommonUtil::getRamUsage();
}
