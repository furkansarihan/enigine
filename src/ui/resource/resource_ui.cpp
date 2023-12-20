#include "resource_ui.h"

const char *blendModeStrings[] = {"Opaque", "Alpha Blend"};

void ResourceUI::render()
{
    if (!ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderModels();
    renderTextures();
}

void ResourceUI::renderModels()
{
    if (!ImGui::TreeNode("Models"))
        return;

    for (auto &it : m_resourceManager->m_models)
    {
        Model *model = it.second;
        if (ImGui::TreeNode(model->m_path.c_str()))
        {
            for (int i = 0; i < model->meshes.size(); i++)
            {
                Mesh &mesh = *model->meshes[i];
                Material &material = *mesh.material;
                renderMaterial(model, material, i);
            }
        }
    }

    ImGui::TreePop();
}

void ResourceUI::renderTextures()
{
    if (!ImGui::TreeNode("Textures"))
        return;

    for (auto &it : m_resourceManager->m_textures)
    {
        const Texture &texture = it.second;
        renderTexture(texture);
    }

    ImGui::TreePop();
}

void ResourceUI::renderMaterial(Model *model, Material &material, int index)
{
    if (!ImGui::TreeNode(std::string(material.name + "##" + std::to_string(index)).c_str()))
        return;

    if (ImGui::Combo("Blend Mode##BlendModeCombo", reinterpret_cast<int *>(&material.blendMode), blendModeStrings, IM_ARRAYSIZE(blendModeStrings)))
        model->updateMeshTypes();
    VectorUI::renderVec4("albedo", material.albedo, 0.1f);
    ImGui::DragFloat("metallic", &material.metallic, 0.1f, 0.f, 1.f);
    ImGui::DragFloat("roughness", &material.roughness, 0.1f, 0.f, 1.f);
    if (ImGui::DragFloat("transmission", &material.transmission, 0.1f, 0.f, 1.f))
        model->updateMeshTypes();
    if (ImGui::DragFloat("opacity", &material.opacity, 0.1f, 0.f, 1.f))
        model->updateMeshTypes();
    ImGui::DragFloat("ior", &material.ior, 0.1f);
    VectorUI::renderVec4("emissiveColor", material.emissiveColor, 0.1f);
    ImGui::DragFloat("emissiveStrength", &material.emissiveStrength, 0.1f);
    ImGui::DragFloat("thickness", &material.thickness, 0.1f);
    ImGui::DragFloat("parallaxMapMidLevel", &material.parallaxMapMidLevel, 0.1f);
    ImGui::DragFloat("parallaxMapSampleCount", &material.parallaxMapSampleCount, 0.1f);
    ImGui::DragFloat("parallaxMapScale", &material.parallaxMapScale, 0.1f);
    ImGui::DragFloat("parallaxMapScaleMode", &material.parallaxMapScaleMode, 0.1f);

    ImGui::BeginTable("material_properties_table", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Value");
    ImGui::TableSetupColumn("Components");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();
    for (const auto &texture : material.textures)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        renderTexture(texture);
        ImGui::TableNextColumn();
        ImGui::Text("%d", texture.nrComponents);
        ImGui::TableNextColumn();
        ImGui::Text("%s", texture.type.c_str());
    }
    ImGui::EndTable();

    ImGui::TreePop();
}

void ResourceUI::renderTexture(const Texture &texture)
{
    float aspectRatio = texture.width / texture.height;
    float desiredWidth = 400.0f;
    float desiredHeight = desiredWidth / aspectRatio;
    ImVec2 size(desiredWidth, desiredHeight);

    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(texture.id)), size);
}
