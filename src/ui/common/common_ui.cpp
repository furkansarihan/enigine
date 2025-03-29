#include "common_ui.h"

void CommonUI::DrawTimerWidget(const Timer &timer, const char *widgetTitle)
{
    if (!ImGui::TreeNode(widgetTitle))
        return;

    const auto &timers = timer.timers;
    // ImGui::Text("Active Timers: %d", (int)timers.size());

    // Calculate global max value for consistent scaling across all graphs
    float globalMax = 0;
    for (const auto &[name, data] : timers)
    {
        for (const auto &duration : data.durationHistory)
            globalMax = std::max(globalMax, (float)duration);
    }

    // Compact table layout
    if (ImGui::BeginTable("TimerTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
    {
        // Table headers
        ImGui::TableSetupColumn("History", ImGuiTableColumnFlags_WidthFixed, 120.f);
        ImGui::TableSetupColumn("Average (µs)");
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();

        // Table content
        for (const auto &[name, data] : timers)
        {
            ImGui::TableNextRow();

            // History graph column
            ImGui::TableSetColumnIndex(0);
            float values[data.durationHistory.size()];
            for (size_t i = 0; i < data.durationHistory.size(); i++)
            {
                values[i] = (float)data.durationHistory[i];
            }

            ImGui::PushID(name.c_str());
            ImGui::PlotLines(
                "##history",
                values,
                (int)data.durationHistory.size(),
                0,
                nullptr,
                0.0f,
                globalMax,
                ImVec2(120.f, 20.f));
            ImGui::PopID();

            // Last duration column
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1f", timer.getAverageDuration(name));

            // Name column
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", name.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::TreePop();
}
