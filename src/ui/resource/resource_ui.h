#ifndef resource_ui_hpp
#define resource_ui_hpp

#include "../../resource_manager/resource_manager.h"
#include "../base_ui.h"

class ResourceUI : public BaseUI
{
private:
    ResourceManager *m_resourceManager;

public:
    ResourceUI(ResourceManager *resourceManager)
        : m_resourceManager(resourceManager)
    {
    }

    void render() override;
    void renderModels();
    void renderTextures();
    bool renderMaterial(Material &material);
    void renderTexture(Texture &texture);
};

#endif /* resource_ui_hpp */
