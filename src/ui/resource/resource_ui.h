#ifndef resource_ui_hpp
#define resource_ui_hpp

#include "../base_ui.h"
#include "../../resource_manager/resource_manager.h"

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
    void renderMaterial(Model *model, Material &material, int index);
    void renderTexture(const Texture &texture);
};

#endif /* resource_ui_hpp */
