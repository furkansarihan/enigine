#ifndef temp_ui_hpp
#define temp_ui_hpp

#include "../base_ui.h"
#include "../../post_process/post_process.h"
#include "../../physics_world/debug_drawer/debug_drawer.h"
#include "../../shader_manager/shader_manager.h"

class TempUI : public BaseUI
{
private:
    PostProcess *m_postProcess;
    DebugDrawer *m_debugDrawer;
    ShaderManager *m_shaderManager;

public:
    TempUI(PostProcess *postProcess, DebugDrawer *debugDrawer, ShaderManager* shaderManager)
        : m_postProcess(postProcess),
          m_debugDrawer(debugDrawer),
          m_shaderManager(shaderManager)
    {
    }

    float m_lightColor[3] = {0.1f, 0.1f, 0.1f};
    float m_ambientColor[3] = {0.4f, 0.4f, 0.4f};
    float m_specularColor[3] = {0.15f, 0.15f, 0.15f};
    float m_lightPower = 10.0;
    float m_radius = 12.0;
    float m_speed = 2.0;

    void render() override;
};

#endif /* temp_ui_hpp */
