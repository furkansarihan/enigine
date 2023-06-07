#ifndef particle_ui_hpp
#define particle_ui_hpp

#include <mach/mach.h>

#include "../base_ui.h"
#include "../../particle_engine/particle_engine.h"

class ParticleUI : public BaseUI
{
public:
    ParticleUI() {}
    
    std::vector<ParticleEngine *> m_particleEngines;

    void render() override;
    void renderParticleEngine(ParticleEngine *pe, int index);
};

#endif /* particle_ui_hpp */
