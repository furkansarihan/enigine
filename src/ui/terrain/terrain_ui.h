#ifndef terrain_ui_hpp
#define terrain_ui_hpp

#include "../base_ui.h"
#include "../../terrain/terrain.h"

class TerrainUI : public BaseUI
{
private:
    Terrain *m_terrain;

public:
    TerrainUI(Terrain *terrain) : m_terrain(terrain) {}

    void render() override;
};

#endif /* terrain_ui_hpp */
