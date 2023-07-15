#include "resource_ui.h"

void ResourceUI::render()
{
    if (!ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    for (auto &it : m_resourceManager->m_models)
    {
        Model *model = it.second;
        if (ImGui::TreeNode(model->m_path.c_str()))
        {
            for (int i = 0; i < model->meshes.size(); i++)
            {
                Mesh &mesh = model->meshes[i];

                if (ImGui::TreeNode(std::string(mesh.name + ":" + mesh.material.name).c_str()))
                {
                    ImGui::BeginTable("material_properties_table", 3, ImGuiTableFlags_Borders);
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();

                    for (const auto &property : mesh.material.properties)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", property.name.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", property.type);
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", property.value.c_str());
                    }

                    ImGui::EndTable();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
}
