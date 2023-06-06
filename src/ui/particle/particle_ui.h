#ifndef particle_ui_hpp
#define particle_ui_hpp

#include <mach/mach.h>

#include "../base_ui.h"
#include "../../particle_engine/particle_engine.h"

class ParticleUI : public BaseUI
{
private:
    ParticleEngine *m_particleEngine;

public:
    ParticleUI(ParticleEngine *particleEngine) : m_particleEngine(particleEngine) {}

    float m_followDist = 3.f;
    float m_followOffsetY = 3.f;

    void render() override;
};

#endif /* particle_ui_hpp */
