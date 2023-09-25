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
                Mesh &mesh = *model->meshes[i];

                if (ImGui::TreeNode(std::string(mesh.name + ":" + mesh.material.name).c_str()))
                {
                    ImGui::BeginTable("material_properties_table", 3, ImGuiTableFlags_Borders);
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();

                    for (auto &property : mesh.material.properties)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", property.name.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", property.type);
                        ImGui::TableNextColumn();
                        if (property.type == aiPTI_Float)
                        {
                            float value = std::stof(property.value);
                            if (ImGui::DragFloat(property.name.c_str(), &value, 0.1f))
                                property.value = std::to_string(value);
                        }
                        else if (property.type == aiPTI_Integer)
                        {
                            int value = std::stoi(property.value);
                            if (ImGui::DragInt(property.name.c_str(), &value))
                                property.value = std::to_string(value);
                        }
                    }

                    for (const auto &texture : mesh.material.textures)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", texture.nrComponents);
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", texture.type.c_str());
                        ImGui::TableNextColumn();

                        float aspectRatio = texture.width / texture.height;
                        float desiredWidth = 400.0f;
                        float desiredHeight = desiredWidth / aspectRatio;
                        ImVec2 size(desiredWidth, desiredHeight);

                        ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(texture.id)), size);
                    }

                    ImGui::EndTable();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
}
