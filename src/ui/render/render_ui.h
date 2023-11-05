#ifndef render_ui_hpp
#define render_ui_hpp

#include "../base_ui.h"
#include "../../render_manager/render_manager.h"

#include <sstream>

class RenderUI : public BaseUI
{
private:
    GLFWwindow *m_window;
    RenderManager *m_renderManager;
    ResourceManager *m_resourceManager;

public:
    RenderUI(GLFWwindow *window, RenderManager *renderManager, ResourceManager *resourceManager)
        : m_window(window),
          m_renderManager(renderManager),
          m_resourceManager(resourceManager),
          m_selectedSource(nullptr),
          m_followSelectedSource(false),
          m_lastSelectScreenPosition(glm::vec2(0.f, 0.f)),
          m_selectDepth(0),
          m_lastInputAt(0.f),
          m_drawNormals(false),
          m_normalSize(0.01f),
          m_drawNormalSource(nullptr),
          m_followDistance(8.f),
          m_followOffset(glm::vec3(0.f, 2.f, 0.f))
    {
    }

    RenderSource *m_selectedSource;
    bool m_followSelectedSource;
    glm::vec2 m_lastSelectScreenPosition;
    int m_selectDepth;
    float m_lastInputAt;
    // normals
    bool m_drawNormals;
    float m_normalSize;
    RenderSource *m_drawNormalSource;
    std::vector<GLfloat> m_normalVertices;
    std::vector<GLuint> m_normalIndices;
    // camera follow
    float m_followDistance;
    glm::vec3 m_followOffset;

    void render() override;
    void inputListener();
    bool canProcessInput();
    void mouseSelect(double xpos, double ypos);
    void selectSource(RenderSource *source);

    void drawSelectedSource(Shader &simpleShader, glm::mat4 mvp);
    void drawSelectedNormals(Shader &lineShader, glm::mat4 mvp, unsigned int vbo, unsigned int vao, unsigned int ebo);

    void renderRenderSource(RenderSource *source);
    void renderSelectedSourceWindow();
    void renderRenderSources();
    void renderLightSources();
    void renderDebug();
    void renderGBuffer();
    void renderSSAO();
    void renderSSAOTextures();
    void renderPostProcess();
    void renderBloom();
    void renderToneMapping();

private:
    void setupDrawNormals();
};

#endif /* render_ui_hpp */
