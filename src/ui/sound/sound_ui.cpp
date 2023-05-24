#include "sound_ui.h"

void SoundUI::render()
{
    if (!ImGui::CollapsingHeader("Sound", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ALint state = m_soundEngine->getSourceState(*m_soundSource);
    if (state == AL_PLAYING)
    {
        if (ImGui::Button("state: playing"))
        {
            m_soundEngine->pauseSource(*m_soundSource);
        }
    }
    else if (state == AL_PAUSED)
    {
        if (ImGui::Button("state: paused"))
        {
            m_soundEngine->playSource(*m_soundSource);
        }
    }
    else if (state == AL_STOPPED)
    {
        if (ImGui::Button("state: stopped"))
        {
            m_soundEngine->playSource(*m_soundSource);
        }
    }
    else
    {
        ImGui::Text("state: unknown");
    }
    ImGui::SameLine();
    if (ImGui::Button("reset"))
    {
        m_soundEngine->setSourceGain(*m_soundSource, 1.0f);
        m_soundEngine->setSourcePitch(*m_soundSource, 1.0f);
    }
    ALfloat gain = m_soundEngine->getSourceGain(*m_soundSource);
    if (ImGui::SliderFloat("gain", &gain, 0.0f, 1.0f, "%.3f"))
    {
        m_soundEngine->setSourceGain(*m_soundSource, gain);
    }
    ALfloat pitch = m_soundEngine->getSourcePitch(*m_soundSource);
    if (ImGui::SliderFloat("pitch", &pitch, 0.5f, 2.0f, "%.3f"))
    {
        m_soundEngine->setSourcePitch(*m_soundSource, pitch);
    }
    ALint looping = m_soundEngine->getSourceLooping(*m_soundSource);
    bool isLooping = looping == AL_TRUE;
    if (ImGui::Checkbox("looping", &isLooping))
    {
        m_soundEngine->setSourceLooping(*m_soundSource, looping ? AL_FALSE : AL_TRUE);
    }
}
