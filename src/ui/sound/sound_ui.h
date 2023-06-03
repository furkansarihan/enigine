#ifndef sound_ui_hpp
#define sound_ui_hpp

#include "../base_ui.h"
#include "../../sound_engine/sound_engine.h"
#include "../../character/playable_character.h"

class SoundUI : public BaseUI
{
private:
    SoundEngine *m_soundEngine;
    PCharacter *m_character;

public:
    SoundUI(SoundEngine *soundEngine, PCharacter *character)
        : m_soundEngine(soundEngine),
          m_character(character)
    {
    }

    void render() override;
    void renderSoundSource(SoundSource &soundSource);
};

#endif /* sound_ui_hpp */
