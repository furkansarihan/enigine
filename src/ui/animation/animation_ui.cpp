#include "animation_ui.h"

void AnimationUI::render()
{
    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    int max = m_animator->m_animations.size() - 1;
    for (int i = 0; i < m_animator->m_state.animations.size(); i++)
    {
        // ImGui::DragInt("index", &m_animator->m_state.animations[i].index, 1, 0, max);
        // ImGui::DragFloat("blendFactor", &m_animator->m_state.animations[i].blendFactor, 0.01f, 0.0f, 1.0f);
        ImGui::Text("index: %d, blendFactor: %.3f", m_animator->m_state.animations[i].index, m_animator->m_state.animations[i].blendFactor);
    }
    ImGui::Separator();
    ImGui::DragInt("selectedAnimPose", &m_selectedAnimPose, 1, 0, m_animator->m_state.poses.size() - 1);
    if (m_selectedAnimPose >= 0 && m_selectedAnimPose < m_animator->m_state.poses.size())
    {
        AnimPose *animPose = &m_animator->m_state.poses[m_selectedAnimPose];
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
