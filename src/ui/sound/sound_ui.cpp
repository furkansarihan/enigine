#include "sound_ui.h"

void SoundUI::render()
{
    if (!ImGui::CollapsingHeader("Sound", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    for (int i = 0; i < m_character->m_fireSounds.size(); i++)
        renderSoundSource(m_character->m_fireSounds[i]);
}

void SoundUI::renderSoundSource(SoundSource &soundSource)
{
    ALint state = m_soundEngine->getSourceState(soundSource);
    if (state == AL_PLAYING)
    {
        if (ImGui::Button("state: playing"))
        {
            m_soundEngine->pauseSource(soundSource);
        }
    }
    else if (state == AL_PAUSED)
    {
        if (ImGui::Button("state: paused"))
        {
            m_soundEngine->playSource(soundSource);
        }
    }
    else if (state == AL_STOPPED)
    {
        if (ImGui::Button("state: stopped"))
        {
            m_soundEngine->playSource(soundSource);
        }
    }
    else
    {
        ImGui::Text("state: unknown");
    }
    ImGui::SameLine();
    if (ImGui::Button("reset"))
    {
        m_soundEngine->setSourceGain(soundSource, 1.0f);
        m_soundEngine->setSourcePitch(soundSource, 1.0f);
    }
    ALfloat gain = m_soundEngine->getSourceGain(soundSource);
    if (ImGui::SliderFloat("gain", &gain, 0.0f, 1.0f, "%.3f"))
    {
        m_soundEngine->setSourceGain(soundSource, gain);
    }
    ALfloat pitch = m_soundEngine->getSourcePitch(soundSource);
    if (ImGui::SliderFloat("pitch", &pitch, 0.5f, 2.0f, "%.3f"))
    {
        m_soundEngine->setSourcePitch(soundSource, pitch);
    }
    ALint looping = m_soundEngine->getSourceLooping(soundSource);
    bool isLooping = looping == AL_TRUE;
    if (ImGui::Checkbox("looping", &isLooping))
    {
        m_soundEngine->setSourceLooping(soundSource, looping ? AL_FALSE : AL_TRUE);
    }
}
