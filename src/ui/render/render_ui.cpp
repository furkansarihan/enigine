#include "render_ui.h"

void RenderUI::render()
{
    if (!ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderGBuffer();
    renderSSAO();

    int totalSourceCount = m_renderManager->m_pbrSources.size();
    int visibleSourceCount = m_renderManager->m_visiblePbrSources.size() + m_renderManager->m_visiblePbrAnimSources.size();
    ImGui::Text("total source count: %d", totalSourceCount);
    ImGui::Text("visible source count: %d", visibleSourceCount);
    ImGui::Checkbox("m_debugCulling", &m_renderManager->m_debugCulling);
    ImGui::Checkbox("m_cullFront", &m_renderManager->m_cullFront);
    // VectorUI::renderVec3("m_shadowBias", m_renderManager->m_shadowBias, 0.001f);
    if (ImGui::Checkbox("m_drawCullingAabb", &m_renderManager->m_drawCullingAabb))
    {
        m_renderManager->m_cullingManager->m_debugDrawer->setDebugMode(m_renderManager->m_drawCullingAabb ? btIDebugDraw::DBG_DrawWireframe
                                                                                                          : btIDebugDraw::DBG_NoDebug);
    }
    ImGui::Checkbox("m_lightAreaDebug", &m_renderManager->m_lightAreaDebug);
    ImGui::Checkbox("m_lightSurfaceDebug", &m_renderManager->m_lightSurfaceDebug);
    ImGui::DragFloat("fogMaxDist", &m_renderManager->fogMaxDist, 100.0f);
    ImGui::DragFloat("fogMinDist", &m_renderManager->fogMinDist, 100.0f);
    ImGui::ColorEdit4("fogColor", &m_renderManager->fogColor[0]);
    ImGui::Text("SSAO");
    // ImGui::DragInt("noiseSize", &m_renderManager->m_ssao->noiseSize, 1);
    ImGui::DragInt("kernelSize", &m_renderManager->m_ssao->kernelSize, 1);
    ImGui::DragFloat("radius", &m_renderManager->m_ssao->radius, 0.001f);
    ImGui::DragFloat("bias", &m_renderManager->m_ssao->bias, 0.001f);
    ImGui::DragFloat("strength", &m_renderManager->m_ssao->strength, 0.001f);
    ImGui::Text("Bloom");
    ImGui::Checkbox("m_karisAverageOnDownsample", &m_renderManager->m_bloomManager->m_karisAverageOnDownsample);
    // ImGui::DragFloat("m_threshold", &m_renderManager->m_bloomManager->m_threshold, 0.01f);
    // ImGui::DragFloat("m_softThreshold", &m_renderManager->m_bloomManager->m_softThreshold, 0.01f);
    ImGui::DragFloat("m_filterRadius", &m_renderManager->m_bloomManager->m_filterRadius, 0.001f);
    ImGui::Text("Post process");
    ImGui::DragFloat("m_contrastBright", &m_renderManager->m_postProcess->m_contrastBright, 0.01f);
    ImGui::DragFloat("m_contrastDark", &m_renderManager->m_postProcess->m_contrastDark, 0.01f);
    ImGui::DragFloat("m_bloomIntensity", &m_renderManager->m_postProcess->m_bloomIntensity, 0.01f);
    if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_sunColor", m_renderManager->m_sunColor, 1.f);
        ImGui::DragFloat("m_sunIntensity", &m_renderManager->m_sunIntensity, 0.1f);
    }
    // list render sources
    if (ImGui::CollapsingHeader("PBR Sources", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        for (const auto &source : m_renderManager->m_pbrSources)
            renderRenderSource(source);
    }

    renderLightSources();

    // if (ImGui::CollapsingHeader("Particle Sources", ImGuiTreeNodeFlags_DefaultOpen))
    // {
    //     for (const auto &source : m_renderManager->m_particleSources)
    //         renderRenderSource(source);
    // }

    renderSelectedSource();
}

void RenderUI::renderRenderSource(RenderSource *source)
{
    std::stringstream ss;
    ss << source;
    if (source->cullIndex != -1)
    {
        for (int i = 0; i < m_renderManager->m_shadowManager->m_sceneObjects[source->cullIndex].frustumIndexes.size(); i++)
        {
            ss << ", ";
            ss << m_renderManager->m_shadowManager->m_sceneObjects[source->cullIndex].frustumIndexes[i];
        }
    }

    if (m_selectedSource == source)
    {
        ImGui::Text("%s", ss.str().c_str());
        return;
    }

    if (ImGui::Button(ss.str().c_str()))
    {
        m_selectedSource = source;
    }
}

void RenderUI::renderSelectedSource()
{
    if (!m_selectedSource)
        return;

    std::stringstream ss;
    ss << m_selectedSource;

    bool m_createNewWindow = true;

    static int corner = 1;
    ImGuiIO &io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x : 0, (corner & 2) ? io.DisplaySize.y : 0);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (ImGui::Begin("renderSelectedSource", &m_createNewWindow, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("%s", ss.str().c_str());
        VectorUI::renderTransform((ss.str() + ":transform").c_str(), m_selectedSource->transform, 0.001f, 1.f, 0.001f);
        ImGui::Separator();
        VectorUI::renderTransform((ss.str() + ":offset").c_str(), m_selectedSource->offset, 0.001f, 1.f, 0.001f);
        if (ImGui::Button("Focus"))
        {
            m_renderManager->m_camera->position = CommonUtil::positionFromModel(m_selectedSource->transform.getModelMatrix());
        }

        ImGui::End();
    }
}

void RenderUI::renderLightSources()
{
    if (!ImGui::CollapsingHeader("Point Light Sources", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::BeginTable("PointLightsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    // Table header
    ImGui::TableSetupColumn("Point Light Index");
    ImGui::TableSetupColumn("Linear");
    ImGui::TableSetupColumn("Quadratic");
    ImGui::TableHeadersRow();

    // Checkbox and input fields
    for (int i = 0; i < m_renderManager->m_pointLights.size(); i++)
    {
        ImGui::TableNextRow();

        LightSource &light = m_renderManager->m_pointLights[i];

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("m_pointLights: %d", i);

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat(("##Linear" + std::to_string(i)).c_str(), &light.linear);

        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat(("##Quadratic" + std::to_string(i)).c_str(), &light.quadratic);
    }

    ImGui::EndTable();
}

void RenderUI::renderGBuffer()
{
    if (!ImGui::CollapsingHeader("G-Buffer", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);
    float width = m_renderManager->m_gBuffer->m_width;
    float height = m_renderManager->m_gBuffer->m_height;
    float aspectRatio = width / height;
    float desiredWidth = 200.0f;
    float desiredHeight = desiredWidth / aspectRatio;

    ImVec2 size(desiredWidth, desiredHeight);

    // Begin the table
    ImGui::BeginTable("G-Buffer-Table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    // Table header
    ImGui::TableSetupColumn("G-Buffer Position");
    ImGui::TableSetupColumn("G-Buffer Normal");
    ImGui::TableSetupColumn("G-Buffer Albedo");
    ImGui::TableSetupColumn("G-Buffer AO-Rough-Metal");
    ImGui::TableHeadersRow();

    // Image and label cells
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gPosition)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gNormalShadow)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(2);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gAlbedo)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(3);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gAoRoughMetal)), size, uv0, uv1);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gViewPosition)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gViewNormal)), size, uv0, uv1);

    // End the table
    ImGui::EndTable();
}

void RenderUI::renderSSAO()
{
    if (!ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);
    float width = m_renderManager->m_gBuffer->m_width;
    float height = m_renderManager->m_gBuffer->m_height;
    float aspectRatio = width / height;
    float desiredWidth = 400.0f;
    float desiredHeight = desiredWidth / aspectRatio;
    ImVec2 size(desiredWidth, desiredHeight);

    ImGui::BeginTable("SSAO-Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
    ImGui::TableSetupColumn("SSAO Color");
    ImGui::TableSetupColumn("SSAO Blur");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_ssao->ssaoColorBuffer)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_ssao->ssaoColorBufferBlur)), size, uv0, uv1);

    // End the table
    ImGui::EndTable();
}
