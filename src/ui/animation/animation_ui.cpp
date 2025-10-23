#include "animation_ui.h"
#include <glm/geometric.hpp>
#include <imgui.h>

AnimationUI::AnimationUI(Animator *animator)
    : m_animator(animator),
      m_selectedAnimation(nullptr)
{
}

void AnimationUI::render()
{
    ImGui::PushID(m_animator);

    if (!ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::PopID();
        return;
    }

    renderBlendTable("State Anims", m_animator->m_state.animations);
    renderBlendTable("Pose Anims", m_animator->m_state.poses);
    renderSelectedAnimation();

    ImGui::PopID();
}

void AnimationUI::renderBlendTable(const std::string &tableName, std::vector<Anim *> &anims)
{
    if (!ImGui::TreeNode(tableName.c_str()))
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

        ImGui::PushID(anim);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", animation->m_name.c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##m_blendFactor", &anim->m_blendFactor, 0.01f, 0.0f);
        ImGui::TableSetColumnIndex(2);
        ImGui::DragFloat("##m_timer", &anim->m_timer, 1.f);
        ImGui::TableSetColumnIndex(3);
        ImGui::Checkbox("##m_timerActive", &anim->m_timerActive);
        ImGui::TableSetColumnIndex(4);
        ImGui::DragFloat("##m_playbackSpeed", &anim->m_playbackSpeed, 0.01f, 0.0f);

        ImGui::PopID();
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

        auto i = std::find_if(
            m_boneNames.begin(),
            m_boneNames.end(),
            [&](const std::string &name) { return bone->m_name.find(name) != std::string::npos; });
        if (i == m_boneNames.end())
            continue;

        ImGui::PushID(bone);
        ImGui::Text("%s", bone->m_name.c_str());
        ImGui::DragFloat("Blend", &bone->m_blendFactor, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Position", &bone->m_positions[0].value[0], 0.001f);
        // if (ImGui::DragFloat4("Rotation", &bone->m_rotations[0].value.w, 0.001f))
        //     bone->m_rotations[0].value = glm::normalize(bone->m_rotations[0].value);
        ImGui::DragFloat4("Rotation", &bone->m_rotations[0].value[0], 0.001f);

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::TreePop();
}
