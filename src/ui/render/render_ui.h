#ifndef render_ui_hpp
#define render_ui_hpp

#include "../base_ui.h"
#include "../../render_manager/render_manager.h"
#include "../../input_manager/input_manager.h"
#include "../../update_manager/update_manager.h"
#include "../../external/imfilebrowser/imfilebrowser.h"

#include <sstream>
#include <functional>

class RenderUI : public BaseUI, public Updatable
{
public:
    RenderUI(InputManager *inputManager, RenderManager *renderManager, ResourceManager *resourceManager);

    InputManager *m_inputManager;
    RenderManager *m_renderManager;
    ResourceManager *m_resourceManager;

    ImGui::FileBrowser m_fileDialog;

    RenderSource *m_selectedSource;
    bool m_drawSelectedSourceAABB;
    bool m_followSelectedSource;
    glm::vec2 m_lastSelectScreenPosition;
    int m_selectDepth;
    // normals
    bool m_drawNormals;
    float m_normalSize;
    RenderSource *m_drawNormalSource;
    std::vector<GLfloat> m_normalVertices;
    std::vector<GLuint> m_normalIndices;
    // armature
    bool m_drawArmature;
    bool m_drawArmatureInFront;
    float m_boneScale;
    // camera follow
    float m_followDistance;
    glm::vec3 m_followOffset;
    // G-Buffer
    float m_desiredWidth = 200.0f;

    void render() override;
    void update(float deltaTime) override;

    void keyListener(GLFWwindow *window, int key, int scancode, int action, int mods);
    void mouseButtonListener(GLFWwindow *window, int button, int action, int mods);
    void mouseScrollListener(GLFWwindow *window, double xoffset, double yoffset);
    void fileDropListener(GLFWwindow *window, int count, const char **paths);
    void mouseSelect(double xpos, double ypos);
    void selectSource(RenderSource *source);

    void renderAddRenderSource();

    void drawSelectedSource(Shader &simpleShader, glm::mat4 mvp);
    void drawSelectedNormals(Shader &lineShader, glm::mat4 mvp, unsigned int vbo, unsigned int vao, unsigned int ebo);
    void drawSelectedArmature(Shader &simpleShader);

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
    void updateEnvironmentTexture(std::string path);
    void addRenderSource(std::string path);
};

#endif /* render_ui_hpp */
