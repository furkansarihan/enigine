#include "render_ui.h"

void RenderUI::render()
{
    if (!ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    int totalSourceCount = m_renderManager->m_pbrSources.size() + m_renderManager->m_basicSources.size();
    int visibleSourceCount = m_renderManager->m_visiblePbrSources.size() + m_renderManager->m_visibleBasicSources.size();
    ImGui::Text("total source count: %d", totalSourceCount);
    ImGui::Text("visible source count: %d", visibleSourceCount);
    ImGui::Checkbox("m_debugCulling", &m_renderManager->m_debugCulling);
    if (ImGui::Checkbox("m_drawCullingAabb", &m_renderManager->m_drawCullingAabb))
    {
        m_renderManager->m_cullingManager->m_debugDrawer->setDebugMode(m_renderManager->m_drawCullingAabb ? btIDebugDraw::DBG_DrawWireframe
                                                                                                        : btIDebugDraw::DBG_NoDebug);
    }
    if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_sunColor", m_renderManager->m_sunColor, 1.f);
        ImGui::DragFloat("m_sunIntensity", &m_renderManager->m_sunIntensity, 0.1f);
    }

    // list render sources
    if (ImGui::CollapsingHeader("PBR Sources", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto &source : m_renderManager->m_pbrSources)
            renderRenderSource(source);
    }

    if (ImGui::CollapsingHeader("Basic Sources", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (const auto &source : m_renderManager->m_basicSources)
            renderRenderSource(source);
    }

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
