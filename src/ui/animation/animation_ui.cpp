#include "animation_ui.h"

void AnimationUI::render()
{
    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    int max = m_animator->m_animations.size() - 1;
    ImGui::DragInt("fromIndex", &m_animator->m_state.fromIndex, 1, 0, max);
    ImGui::DragInt("toIndex", &m_animator->m_state.toIndex, 1, 0, max);
    ImGui::DragFloat("blendFactor", &m_animator->m_state.blendFactor, 0.01f, 0.0f, 1.0f);
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
