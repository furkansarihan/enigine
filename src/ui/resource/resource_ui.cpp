#include "resource_ui.h"

const char *blendModeStrings[] = {"Opaque", "Alpha Blend"};

void ResourceUI::render()
{
    if (!ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderModels();
    /* for (auto &it : m_resourceManager->m_materials)
    {
        Material *material = it.second;
        renderMaterial(*material);
    } */
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
                if (renderMaterial(material))
                    model->updateMeshTypes();
            }

            ImGui::TreePop();
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
        Texture &texture = it.second;
        renderTexture(texture);
    }

    ImGui::TreePop();
}

bool ResourceUI::renderMaterial(Material &material)
{
    bool alphaUpdated = false;

    ImGui::PushID(&material);
    if (!ImGui::TreeNode(material.name.c_str()))
    {
        ImGui::PopID();
        return alphaUpdated;
    }

    if (ImGui::Combo("Blend Mode##BlendModeCombo", reinterpret_cast<int *>(&material.blendMode), blendModeStrings, IM_ARRAYSIZE(blendModeStrings)))
        alphaUpdated = true;
    ImGui::DragFloat2("uvScale", &material.uvScale.x, 0.01f, 0.f);
    ImGui::ColorEdit4("albedo", &material.albedo.x);
    ImGui::DragFloat("metallic", &material.metallic, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("roughness", &material.roughness, 0.01f, 0.f, 1.f);
    if (ImGui::DragFloat("transmission", &material.transmission, 0.01f, 0.f, 1.f))
        alphaUpdated = true;
    if (ImGui::DragFloat("opacity", &material.opacity, 0.01f, 0.f, 1.f))
        alphaUpdated = true;
    ImGui::DragFloat("ior", &material.ior, 0.01f, 0.f, 100.f);
    ImGui::ColorEdit4("emissiveColor", &material.emissiveColor.x);
    ImGui::DragFloat("emissiveStrength", &material.emissiveStrength, 0.01f, 0.f, 100.f);
    ImGui::DragFloat("thickness", &material.thickness, 0.01f);
    ImGui::DragFloat("parallaxMapMidLevel", &material.parallaxMapMidLevel, 0.01f);
    ImGui::DragFloat("parallaxMapSampleCount", &material.parallaxMapSampleCount, 0.01f);
    ImGui::DragFloat("parallaxMapScale", &material.parallaxMapScale, 0.01f);
    ImGui::DragFloat("parallaxMapScaleMode", &material.parallaxMapScaleMode, 1.f, 0.f, 1.f);

    ImGui::BeginTable("material_properties_table", 3, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Value");
    ImGui::TableSetupColumn("Components");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();
    for (auto &texture : material.textures)
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
    ImGui::PopID();

    return alphaUpdated;
}

void ResourceUI::renderTexture(Texture &texture)
{
    float aspectRatio = static_cast<float>(texture.width) / static_cast<float>(texture.height);
    float desiredWidth = 400.0f;
    float desiredHeight = desiredWidth / aspectRatio;
    ImVec2 size(desiredWidth, desiredHeight);

    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(texture.id)), size);
}
