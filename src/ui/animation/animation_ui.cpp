#include "animation_ui.h"

AnimationUI::AnimationUI(Animator *animator)
    : m_animator(animator),
      m_selectedAnimPose(0)
{
}

void AnimationUI::render()
{
    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderBlendTable();
    renderSelectedAnimPose();
}

void AnimationUI::renderBlendTable()
{
    if (!ImGui::TreeNode("Blend Table##AnimationUI::renderBlendTable"))
        return;

    ImGui::BeginTable("AnimationTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Blend Factor");
    ImGui::TableSetupColumn("Playback Speed");
    ImGui::TableHeadersRow();
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
        ImGui::TableSetColumnIndex(2);
        ImGui::DragFloat(("##m_playbackSpeed:" + m_animator->m_animations[i]->m_name).c_str(), &m_animator->m_animations[i]->m_playbackSpeed, 0.01f, 0.0f);
    }
    ImGui::EndTable();

    ImGui::TreePop();
}

void AnimationUI::renderSelectedAnimPose()
{
    ImGui::DragInt("AnimPose Index##AnimationUI::renderSelectedAnimPose", &m_selectedAnimPose, 1, 0, m_animator->m_state.poses.size() - 1);

    if (m_animator->m_state.poses.empty())
        return;

    AnimPose *animPose = &m_animator->m_state.poses[m_selectedAnimPose];
    ImGui::Text("Name: %s, Index: %d", m_animator->m_animations[animPose->index]->m_name.c_str(), animPose->index);
    ImGui::DragFloat("Blend Factor##AnimationUI::renderSelectedAnimPose", &animPose->blendFactor, 0.01f, 0.0f, 1.0f);

    if (!ImGui::TreeNode("Selected AnimPose##AnimationUI::renderSelectedAnimPose"))
        return;

    auto bonesMap = &m_animator->m_animations[animPose->index]->m_bones;
    for (auto it = bonesMap->begin(); it != bonesMap->end(); ++it)
    {
        Bone *bone = it->second;
        ImGui::DragFloat(bone->m_name.c_str(), &bone->m_blendFactor, 0.01f, 0.0f, 1.0f);
    }

    ImGui::TreePop();
}
