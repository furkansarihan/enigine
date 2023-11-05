#ifndef root_ui_hpp
#define root_ui_hpp

#include <string>
#include <vector>

#include "base_ui.h"

#include "animation/animation_ui.h"
#include "camera/camera_ui.h"
#include "character/character_ui.h"
#include "ragdoll/ragdoll_ui.h"
#include "shadowmap/shadowmap_ui.h"
#include "sound/sound_ui.h"
#include "system_monitor/system_monitor_ui.h"
#include "terrain/terrain_ui.h"
#include "vehicle/vehicle_ui.h"
#include "particle/particle_ui.h"
#include "resource/resource_ui.h"
#include "render/render_ui.h"
#include "physics/physics_ui.h"

class RootUI
{
public:
    RootUI();
    ~RootUI();

    std::vector<BaseUI *> m_uiList;

    void render();
};

#endif /* root_ui_hpp */
