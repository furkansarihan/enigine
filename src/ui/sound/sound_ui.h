#ifndef sound_ui_hpp
#define sound_ui_hpp

#include "../base_ui.h"
#include "../../sound_engine/sound_engine.h"

class SoundUI : public BaseUI
{
private:
    SoundEngine *m_soundEngine;
    SoundSource *m_soundSource;

public:
    SoundUI(SoundEngine *soundEngine, SoundSource *soundSource)
        : m_soundEngine(soundEngine),
          m_soundSource(soundSource)
    {
    }

    void render() override;
};

#endif /* sound_ui_hpp */
