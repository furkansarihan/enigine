#ifndef terrain_ui_hpp
#define terrain_ui_hpp

#include "../base_ui.h"
#include "../../terrain/terrain.h"
#include "../../model/model.h"
#include "../../shader/shader.h"

#include <glm/glm.hpp>

class TerrainUI : public BaseUI
{
private:
    Terrain *m_terrain;

public:
    TerrainUI(Terrain *terrain) : m_terrain(terrain) {}

    bool m_debugCulling = false;

    void render() override;
    void drawHeightCells(Shader &shader, Model &cube, glm::mat4 viewProjection);
};

#endif /* terrain_ui_hpp */
