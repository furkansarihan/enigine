#ifndef shadowmap_ui_hpp
#define shadowmap_ui_hpp

#include "../base_ui.h"
#include "../../shadow_manager/shadow_manager.h"
#include "../../shadowmap_manager/shadowmap_manager.h"
#include "../../camera/camera.h"
#include "../../shader/shader.h"

class ShadowmapUI : public BaseUI
{
private:
    ShadowManager *m_shadowManager;
    ShadowmapManager *m_shadowmapManager;

public:
    ShadowmapUI(ShadowManager *shadowManager, ShadowmapManager *shadowmapManager)
        : m_shadowManager(shadowManager),
          m_shadowmapManager(shadowmapManager)
    {
    }

    float m_quadScale = 0.2f;
    bool m_drawFrustum = false;
    bool m_drawAABB = false;
    bool m_drawShadowmap = false;

    void render() override;
    void drawFrustum(Shader &simpleShader, glm::mat4 mvp, unsigned int c_vbo, unsigned int c_vao, unsigned int c_ebo);
    void drawLightAABB(Shader &simpleShader, glm::mat4 mvp, glm::mat4 inverseDepthViewMatrix, unsigned int c_vbo, unsigned int c_vao, unsigned int c_ebo);
    void drawShadowmap(Shader &textureArrayShader, float screenWidth, float screenHeight, unsigned int q_vao);
};

#endif /* shadowmap_ui_hpp */
