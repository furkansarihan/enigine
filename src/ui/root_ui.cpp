#include "root_ui.h"

RootUI::RootUI()
{
}

RootUI::~RootUI()
{
    m_uiList.clear();
}

void RootUI::render()
{
    // start imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static int corner = 0;
    ImGuiIO &io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x : 0, (corner & 2) ? io.DisplaySize.y : 0);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.85f);
    bool p_open = true;
    if (ImGui::Begin("overlay", &p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        for (int i = 0; i < m_uiList.size(); i++)
            m_uiList.at(i)->render();
    }
    ImGui::End();

    // end imgui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
