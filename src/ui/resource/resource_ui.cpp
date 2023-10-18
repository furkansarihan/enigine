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
                    VectorUI::renderVec4("albedo", mesh.material.albedo, 0.1f);
                    ImGui::DragFloat("metallic", &mesh.material.metallic, 0.1f, 0.f, 1.f);
                    ImGui::DragFloat("roughness", &mesh.material.roughness, 0.1f, 0.f, 1.f);
                    ImGui::DragFloat("transmission", &mesh.material.transmission, 0.1f, 0.f, 1.f);
                    ImGui::DragFloat("opacity", &mesh.material.opacity, 0.1f, 0.f, 1.f);
                    ImGui::DragFloat("ior", &mesh.material.ior, 0.1f);
                    VectorUI::renderVec4("emissiveColor", mesh.material.emissiveColor, 0.1f);
                    ImGui::DragFloat("emissiveStrength", &mesh.material.emissiveStrength, 0.1f);
                    ImGui::DragFloat("thickness", &mesh.material.thickness, 0.1f);
                    ImGui::DragFloat("parallaxMapMidLevel", &mesh.material.parallaxMapMidLevel, 0.1f);
                    ImGui::DragFloat("parallaxMapSampleCount", &mesh.material.parallaxMapSampleCount, 0.1f);
                    ImGui::DragFloat("parallaxMapScale", &mesh.material.parallaxMapScale, 0.1f);
                    ImGui::DragFloat("parallaxMapScaleMode", &mesh.material.parallaxMapScaleMode, 0.1f);

                    ImGui::BeginTable("material_properties_table", 3, ImGuiTableFlags_Borders);
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableHeadersRow();

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
