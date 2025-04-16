#include "resource_ui.h"

const char *blendModeStrings[] = {"Opaque", "Alpha Blend"};

void ResourceUI::render()
{
    if (!ImGui::CollapsingHeader("Resource", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    // renderModels();
    for (auto &it : m_resourceManager->m_materials)
    {
        Material *material = it.second;
        renderMaterial(*material);
    }
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

    // TODO: select texture

    for (auto &it : m_resourceManager->m_textures)
    {
        Texture *texture = it.second;
        renderTexture(*texture);
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
        renderTexture(*texture);
        ImGui::TableNextColumn();
        ImGui::Text("%d", texture->nrComponents);
        ImGui::TableNextColumn();
        ImGui::Text("%s", texture->type.c_str());
    }
    ImGui::EndTable();

    ImGui::TreePop();
    ImGui::PopID();

    return alphaUpdated;
}

bool TextureParamsUI(TextureParams &params)
{
    ImGui::PushID(&params);

    bool changed = false;

    // Wrap Mode S
    const char *wrapModeLabels[] = {"Clamp to Edge", "Clamp to Border", "Repeat"};
    int wrapModeS = static_cast<int>(params.wrapModeS);
    if (ImGui::Combo("Wrap Mode S", &wrapModeS, wrapModeLabels, IM_ARRAYSIZE(wrapModeLabels)))
    {
        params.wrapModeS = static_cast<TextureWrapMode>(wrapModeS);
        changed = true;
    }

    // Wrap Mode T
    int wrapModeT = static_cast<int>(params.wrapModeT);
    if (ImGui::Combo("Wrap Mode T", &wrapModeT, wrapModeLabels, IM_ARRAYSIZE(wrapModeLabels)))
    {
        params.wrapModeT = static_cast<TextureWrapMode>(wrapModeT);
        changed = true;
    }

    // Min Filter
    const char *minFilterLabels[] = {"Nearest", "Linear"};
    int minFilter = static_cast<int>(params.minFilter);
    if (ImGui::Combo("Min Filter", &minFilter, minFilterLabels, IM_ARRAYSIZE(minFilterLabels)))
    {
        params.minFilter = static_cast<TextureFilterMode>(minFilter);
        changed = true;
    }

    // Mag Filter
    const char *magFilterLabels[] = {"Nearest", "Linear"};
    int magFilter = static_cast<int>(params.magFilter);
    if (ImGui::Combo("Mag Filter", &magFilter, magFilterLabels, IM_ARRAYSIZE(magFilterLabels)))
    {
        params.magFilter = static_cast<TextureFilterMode>(magFilter);
        changed = true;
    }

    // Anisotropic Filtering
    if (ImGui::Checkbox("Anisotropic Filtering", &params.anisotropicFiltering))
        changed = true;

    // Max Anisotropy
    if (params.anisotropicFiltering)
    {
        if (ImGui::SliderFloat("Max Anisotropy", &params.maxAnisotropy, 1.0f, 16.0f, "%.1f"))
            changed = true;
    }

    // Generate Mipmaps
    if (ImGui::Checkbox("Generate Mipmaps", &params.generateMipmaps))
        changed = true;

    // TODO:
    // Data Type (read-only, for display purposes)
    // const char *dataTypeLabels[] = {"Unsigned Byte", "Float", "Half Float", "Int", "Unsigned Int"};
    // int dataType = static_cast<int>(params.dataType);
    // ImGui::Combo("Data Type", &dataType, dataTypeLabels, IM_ARRAYSIZE(dataTypeLabels));

    ImGui::PopID();

    return changed;
}

void ResourceUI::renderTexture(Texture &texture)
{
    float aspectRatio = static_cast<float>(texture.width) / static_cast<float>(texture.height);
    float desiredWidth = 400.0f;
    float desiredHeight = desiredWidth / aspectRatio;
    ImVec2 size(desiredWidth, desiredHeight);

    ImGui::PushID(&texture);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(texture.id)), size);

    TextureParams newParams = texture.params;
    if (TextureParamsUI(newParams))
        m_resourceManager->updateTexture(texture, newParams);

    ImGui::PopID();
}
