#ifndef render_ui_hpp
#define render_ui_hpp

#include "../base_ui.h"
#include "../../render_manager/render_manager.h"

#include <sstream>

class RenderUI : public BaseUI
{
private:
    RenderManager *m_renderManager;

public:
    RenderUI(RenderManager *renderManager) : m_renderManager(renderManager) {}

    RenderSource *m_selectedSource = nullptr;

    void render() override;
    void renderRenderSource(RenderSource *source);
    void renderSelectedSource();
    void renderLightSources();
    void renderGBuffer();
    void renderSSAO();
};

#endif /* render_ui_hpp */
