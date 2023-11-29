#include "animation_ui.h"

AnimationUI::AnimationUI(Animator *animator)
    : m_animator(animator),
      m_selectedAnimation(nullptr)
{
}

void AnimationUI::render()
{
    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderBlendTable("State Anims", m_animator->m_state.animations);
    renderBlendTable("Pose Anims", m_animator->m_state.poses);
    renderSelectedAnimation();
}

void AnimationUI::renderBlendTable(const std::string &tableName, std::vector<Anim *> &anims)
{
    if (!ImGui::TreeNode((tableName + " Blend Table##AnimationUI::renderBlendTable").c_str()))
        return;

    ImGui::BeginTable("State Animations", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Blend Factor");
    ImGui::TableSetupColumn("Timer");
    ImGui::TableSetupColumn("Timer Active");
    ImGui::TableSetupColumn("Playback Speed");
    ImGui::TableHeadersRow();
    for (int i = 0; i < anims.size(); i++)
    {
        Anim *anim = anims[i];
        Animation *animation = anim->m_animation;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", animation->m_name.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat(("##m_blendFactor:" + animation->m_name).c_str(), &anim->m_blendFactor, 0.01f, 0.0f);
        ImGui::TableSetColumnIndex(2);
        ImGui::DragFloat(("##m_timer:" + animation->m_name).c_str(), &anim->m_timer, 1.f);
        ImGui::TableSetColumnIndex(3);
        ImGui::Checkbox(("##m_timerActive:" + animation->m_name).c_str(), &anim->m_timerActive);
        ImGui::TableSetColumnIndex(4);
        ImGui::DragFloat(("##m_playbackSpeed:" + animation->m_name).c_str(), &anim->m_playbackSpeed, 0.01f, 0.0f);
    }
    ImGui::EndTable();

    ImGui::TreePop();
}

void AnimationUI::renderSelectedAnimation()
{
    if (m_selectedAnimation == nullptr)
        return;

    ImGui::Text("Name: %s", m_selectedAnimation->m_name.c_str());

    if (!ImGui::TreeNode("Bones##AnimationUI::renderSelectedAnimation"))
        return;

    auto bonesMap = &m_selectedAnimation->m_bones;
    for (auto it = bonesMap->begin(); it != bonesMap->end(); ++it)
    {
        Bone *bone = it->second;
        ImGui::DragFloat(bone->m_name.c_str(), &bone->m_blendFactor, 0.01f, 0.0f, 1.0f);
    }

    ImGui::TreePop();
}
