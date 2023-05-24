#ifndef shadowmap_ui_hpp
#define shadowmap_ui_hpp

#include "../base_ui.h"
#include "../../shadow_manager/shadow_manager.h"

class ShadowmapUI : public BaseUI
{
private:
    ShadowManager *m_shadowManager;

public:
    ShadowmapUI(ShadowManager *shadowManager) : m_shadowManager(shadowManager) {}

    float m_quadScale = 0.2f;
    bool m_drawFrustum = false;
    bool m_drawAABB = false;
    bool m_drawShadowmap = false;

    void render() override;
};

#endif /* shadowmap_ui_hpp */
