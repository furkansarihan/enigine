#include "animation_ui.h"

void AnimationUI::render()
{
    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::BeginTable("AnimationTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Blend Factor");
    ImGui::TableHeadersRow();
    // Iterate through each animation and display the data in a row
    for (int i = 0; i < m_animator->m_state.animations.size(); i++)
    {
        // int max = m_animator->m_animations.size() - 1;
        // ImGui::DragInt("index", &m_animator->m_state.animations[i].index, 1, 0, max);
        // ImGui::DragFloat("blendFactor", &m_animator->m_state.animations[i].blendFactor, 0.01f, 0.0f, 1.0f);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        int index = m_animator->m_state.animations[i].index;
        ImGui::Text("%s", m_animator->m_animations[index]->m_name.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%.3f", m_animator->m_state.animations[i].blendFactor);
    }
    ImGui::EndTable();
    ImGui::DragInt("m_selectedAnimPose", &m_selectedAnimPose, 1, 0, m_animator->m_state.poses.size() - 1);
    renderBoneList();
}

void AnimationUI::renderBoneList()
{
    if (!ImGui::CollapsingHeader("Bone List", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;
    if (m_selectedAnimPose >= 0 && m_selectedAnimPose < m_animator->m_state.poses.size())
    {
        AnimPose *animPose = &m_animator->m_state.poses[m_selectedAnimPose];
        ImGui::Text("%s", m_animator->m_animations[animPose->index]->m_name.c_str());
        ImGui::Text("animPose.index: %d", animPose->index);
        ImGui::DragFloat("animPose.blendFactor", &animPose->blendFactor, 0.01f, 0.0f, 1.0f);
        auto bonesMap = &m_animator->m_animations[animPose->index]->m_bones;
        for (auto it = bonesMap->begin(); it != bonesMap->end(); ++it)
        {
            Bone *bone = it->second;
            ImGui::DragFloat(bone->m_Name.c_str(), &bone->m_blendFactor, 0.01f, 0.0f, 1.0f);
        }
    }
}
