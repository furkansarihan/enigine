#include "render_ui.h"

#include <glm/gtc/type_ptr.hpp>

RenderUI::RenderUI(InputManager *inputManager, RenderManager *renderManager, ResourceManager *resourceManager)
    : m_inputManager(inputManager),
      m_renderManager(renderManager),
      m_resourceManager(resourceManager),
      m_selectedSource(nullptr),
      m_followSelectedSource(false),
      m_lastSelectScreenPosition(glm::vec2(0.f, 0.f)),
      m_selectDepth(0),
      m_drawNormals(false),
      m_normalSize(0.01f),
      m_drawNormalSource(nullptr),
      m_followDistance(8.f),
      m_followOffset(glm::vec3(0.f, 2.f, 0.f))
{
    m_inputManager->addKeyListener(std::bind(&RenderUI::keyListener, this,
                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_inputManager->addMouseButtonListener(std::bind(&RenderUI::mouseButtonListener, this,
                                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_inputManager->addMouseScrollListener(std::bind(&RenderUI::mouseScrollListener, this,
                                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_inputManager->addFileDropListener(std::bind(&RenderUI::fileDropListener, this,
                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void RenderUI::render()
{
    if (!ImGui::CollapsingHeader("Render##RenderUI::render", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    // TODO: auto
    if (ImGui::Button("Refresh Shaders"))
    {
        m_renderManager->m_shaderManager->initShaders();
    }

    renderAddRenderSource();
    renderDebug();
    renderGBuffer();
    renderSSAO();
    renderPostProcess();
    renderRenderSources();
    renderLightSources();
    // TODO: particle sources

    renderSelectedSourceWindow();
}

void RenderUI::keyListener(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    GLFWwindow *m_window = m_inputManager->m_window;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        selectSource(nullptr);
    }
    else if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
    {
        if (m_selectedSource)
        {
            m_renderManager->removeSource(m_selectedSource);
            selectSource(nullptr);
        }
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            m_followSelectedSource = false;
        else
            m_followSelectedSource = true;

        m_followDistance = 8.0f;
    }
}

void RenderUI::mouseButtonListener(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        mouseSelect(xpos, ypos);
    }
}

void RenderUI::mouseScrollListener(GLFWwindow *window, double xoffset, double yoffset)
{
    float factor = 1.f;
    m_followDistance += yoffset * factor;
}

void RenderUI::fileDropListener(GLFWwindow *window, int count, const char **paths)
{
    for (int i = 0; i < count; i++)
    {
        const char *path = paths[i];

        addRenderSource(std::string(path));
    }
}

void RenderUI::addRenderSource(std::string path)
{
    eTransform transform;
    transform.setPosition(glm::vec3(0.f, 0.f, 0.f));
    transform.setScale(glm::vec3(1.f));

    Model &model = *m_resourceManager->getModelFullPath(path);
    RenderSource *source = RenderSourceBuilder()
                               .setTransform(transform)
                               .setModel(&model)
                               .build();
    m_renderManager->addSource(source);
    selectSource(source);
}

void RenderUI::renderAddRenderSource()
{
    if (ImGui::Button("Add Render Source"))
    {
        m_fileDialog.Open();
    }

    m_fileDialog.Display();

    if (m_fileDialog.HasSelected())
    {
        std::string path = m_fileDialog.GetSelected().string();

        addRenderSource(path);

        m_fileDialog.ClearSelected();
    }
}

void RenderUI::mouseSelect(double xpos, double ypos)
{
    float screenWidth = m_renderManager->m_screenW;
    float screenHeight = m_renderManager->m_screenH;

    // Obtain the click position in screen coordinates
    glm::vec2 clickScreenPos(xpos, screenHeight - ypos);

    // Get the camera's view and projection matrices
    glm::mat4 viewMatrix = m_renderManager->m_view;
    glm::mat4 projectionMatrix = m_renderManager->m_projection;

    // Unproject the screen coordinates to obtain a point in 3D space
    glm::vec3 rayNear = glm::unProject(glm::vec3(clickScreenPos, 0.0f), viewMatrix, projectionMatrix, glm::vec4(0, 0, screenWidth, screenHeight));
    glm::vec3 rayFar = glm::unProject(glm::vec3(clickScreenPos, 1.0f), viewMatrix, projectionMatrix, glm::vec4(0, 0, screenWidth, screenHeight));

    // Calculate the direction vector from the near plane to the far plane
    glm::vec3 rayDirection = glm::normalize(rayFar - rayNear);

    // Calculate rayTo by adding the direction vector to the camera's position
    glm::vec3 rayFrom = m_renderManager->m_camera->position;
    glm::vec3 rayTo = rayFrom + rayDirection * 500.f;

    std::vector<SelectedObject> objects = m_renderManager->m_cullingManager->getObjects(rayFrom, rayTo);

    if (!objects.empty())
    {
        // increase depth index when clicking same position or the object is already selected
        float distance = glm::distance(m_lastSelectScreenPosition, clickScreenPos);
        if (distance < 1.f || (RenderSource *)objects[m_selectDepth].userPointer == m_selectedSource)
            m_selectDepth++;
        else
            m_selectDepth = 0;

        m_selectDepth = m_selectDepth % objects.size();

        selectSource((RenderSource *)objects[m_selectDepth].userPointer);
    }
    else
    {
        selectSource(nullptr);
        m_selectDepth = 0;
    }

    m_lastSelectScreenPosition = clickScreenPos;
}

void RenderUI::selectSource(RenderSource *source)
{
    m_followSelectedSource = false;
    m_followDistance = 8.0f;
    m_selectedSource = source;
}

// TODO: outline
// TODO: gizmo
void RenderUI::drawSelectedSource(Shader &simpleShader, glm::mat4 mvp)
{
    if (m_selectedSource == nullptr || m_selectedSource->model == nullptr)
        return;

    glm::vec3 center = (m_selectedSource->model->aabbMin + m_selectedSource->model->aabbMax) / 2.f;
    glm::vec3 scale = (m_selectedSource->model->aabbMin - m_selectedSource->model->aabbMax) / 2.f;

    glm::mat4 sourceModel = m_selectedSource->transform.getModelMatrix() * m_selectedSource->offset.getModelMatrix();
    glm::mat4 model = glm::translate(glm::mat4(1.f), center);
    model = glm::scale(model, scale);

    simpleShader.use();
    simpleShader.setMat4("MVP", mvp * sourceModel * model);
    simpleShader.setVec4("DiffuseColor", glm::vec4(1.f, 1.f, 1.f, 1.f));

    // aabb
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_renderManager->cube->draw(simpleShader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // volume
    /* simpleShader.setVec4("DiffuseColor", glm::vec4(1.f, 1.f, 1.f, 0.1f));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_renderManager->cube->draw(simpleShader);
    glDisable(GL_BLEND); */

    // center
    model = glm::translate(glm::mat4(1.f), CommonUtil::positionFromModel(sourceModel) + center);
    model = glm::scale(model, glm::vec3(0.05));

    simpleShader.setMat4("MVP", mvp * model);
    simpleShader.setVec4("DiffuseColor", glm::vec4(1.f, 0.f, 1.f, 1.f));
    glDisable(GL_DEPTH_TEST);
    m_renderManager->sphere->draw(simpleShader);
    glEnable(GL_DEPTH_TEST);
}

void RenderUI::drawSelectedNormals(Shader &lineShader, glm::mat4 mvp, unsigned int vbo, unsigned int vao, unsigned int ebo)
{
    if (!m_drawNormals || m_selectedSource == nullptr)
        return;

    if (m_selectedSource != m_drawNormalSource)
    {
        setupDrawNormals();
        m_drawNormalSource = m_selectedSource;
    }

    glm::mat4 model = m_selectedSource->transform.getModelMatrix() * m_selectedSource->offset.getModelMatrix();
    mvp = mvp * model;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_normalVertices.size() * sizeof(float), m_normalVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_normalIndices.size() * sizeof(float), m_normalIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    lineShader.use();
    lineShader.setMat4("MVP", mvp);

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, m_normalIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void addLine(std::vector<GLfloat> &vertices, std::vector<GLuint> &indices,
             glm::vec3 offset, glm::vec3 start, glm::vec3 end, glm::vec3 color, int indexI)
{
    vertices.push_back(start.x + offset.x);
    vertices.push_back(start.y + offset.z);
    vertices.push_back(start.z + offset.y);

    vertices.push_back(color.x);
    vertices.push_back(color.y);
    vertices.push_back(color.z);

    vertices.push_back(end.x + offset.x);
    vertices.push_back(end.y + offset.z);
    vertices.push_back(end.z + offset.y);

    vertices.push_back(color.x);
    vertices.push_back(color.y);
    vertices.push_back(color.z);

    indices.push_back(indexI);
    indices.push_back(indexI + 1);
}

// TODO: transform agnostic normal size
// TODO: animated model normals
void RenderUI::setupDrawNormals()
{
    m_normalVertices.clear();
    m_normalIndices.clear();

    int indexI = 0;
    int index = 0;
    for (auto &mesh : m_selectedSource->model->meshes)
    {
        index = 0;
        for (auto &vertex : mesh->vertices)
        {
            // if (m_normalIndex != index && m_normalIndex != -1)
            // {
            //     index++;
            //     continue;
            // }

            glm::vec3 position = glm::vec3(mesh->offset * glm::vec4(vertex.position, 1.f));

            glm::vec3 start, end, color;
            // normal
            start = glm::vec3(position);
            end = glm::vec3(position + vertex.normal * m_normalSize);
            color = glm::vec3(0, 1, 0);
            addLine(m_normalVertices, m_normalIndices, m_renderManager->getWorldOrigin(), start, end, color, indexI);
            indexI += 2;
            // tangent
            start = glm::vec3(position);
            end = glm::vec3(position + vertex.tangent * m_normalSize);
            color = glm::vec3(1, 0, 0);
            addLine(m_normalVertices, m_normalIndices, m_renderManager->getWorldOrigin(), start, end, color, indexI);
            indexI += 2;
            // bitangent
            start = glm::vec3(position);
            end = glm::vec3(position + vertex.bitangent * m_normalSize);
            color = glm::vec3(0, 0, 1);
            addLine(m_normalVertices, m_normalIndices, m_renderManager->getWorldOrigin(), start, end, color, indexI);
            indexI += 2;

            index++;
        }
    }
}

void RenderUI::renderRenderSource(RenderSource *source)
{
    std::stringstream ss;
    ss << source;
    if (source->cullIndex != -1)
    {
        for (int i = 0; i < m_renderManager->m_shadowManager->m_sceneObjects[source->cullIndex].frustumIndexes.size(); i++)
        {
            ss << ", ";
            ss << m_renderManager->m_shadowManager->m_sceneObjects[source->cullIndex].frustumIndexes[i];
        }
    }

    if (ImGui::Selectable(ss.str().c_str(), m_selectedSource == source))
    {
        if (m_selectedSource == source)
            selectSource(nullptr);
        else
            selectSource(source);
    }
}

void RenderUI::renderSelectedSourceWindow()
{
    if (!m_selectedSource)
        return;

    std::stringstream ss;
    ss << m_selectedSource;

    bool m_createNewWindow = true;

    // TODO: resolve second window causing frame drop
    // static int corner = 1;
    // ImGuiIO &io = ImGui::GetIO();
    // if (corner != -1)
    // {
    //     ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x : 0, (corner & 2) ? io.DisplaySize.y : 0);
    //     ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    //     ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    // }

    // if (!ImGui::Begin("SelectedSourceWindow", &m_createNewWindow, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    //     return;
    ImGui::Separator();
    ImGui::Text("Selected Render Source");

    if (VectorUI::renderTransform(("transform##" + ss.str()).c_str(), m_selectedSource->transform, 0.001f, 0.001f, 0.001f))
        m_selectedSource->updateModelMatrix();
    if (VectorUI::renderTransform(("offset##" + ss.str()).c_str(), m_selectedSource->offset, 0.001f, 0.001f, 0.001f))
        m_selectedSource->updateModelMatrix();

    ImGui::Checkbox("Follow", &m_followSelectedSource);
    if (m_followSelectedSource)
    {
        glm::vec3 position = CommonUtil::positionFromModel(m_selectedSource->transform.getModelMatrix());
        glm::vec3 followOffset = glm::vec3(0.f);
        float followDistanceBase = 0.f;
        if (m_selectedSource->model != nullptr)
        {
            followOffset.y = (m_selectedSource->model->aabbMax.y - m_selectedSource->model->aabbMin.y) / 4.f;
            followDistanceBase += (m_selectedSource->model->aabbMax.x - m_selectedSource->model->aabbMin.x) / 2.f;
            followDistanceBase += (m_selectedSource->model->aabbMax.z - m_selectedSource->model->aabbMin.z) / 2.f;
        }
        glm::vec3 scale = m_selectedSource->transform.getScale();
        followOffset *= scale;
        followDistanceBase *= (scale.x + scale.z);
        glm::vec3 newPosition = position - m_renderManager->m_camera->front * (followDistanceBase + m_followDistance) + followOffset;
        m_renderManager->m_camera->position = glm::mix(m_renderManager->m_camera->position, newPosition, 0.8f);
    }
    ImGui::Checkbox("Draw Normals", &m_drawNormals);
    if (ImGui::DragFloat("Normal Size", &m_normalSize, 0.001f))
        m_drawNormalSource = nullptr;

    // ImGui::End();
    ImGui::Separator();
}

void RenderUI::renderRenderSources()
{
    if (!ImGui::TreeNode("Render Sources##RenderUI::renderRenderSources"))
        return;

    float maxHeight = 400.0f;
    float height = ImGui::GetFrameHeight() * m_renderManager->m_pbrSources.size();
    if (height > maxHeight)
        height = maxHeight;

    ImGui::BeginChild("scrollableArea##RenderUI::renderRenderSources", ImVec2(400, height), true);

    for (const auto &source : m_renderManager->m_pbrSources)
        renderRenderSource(source);

    ImGui::EndChild();

    ImGui::TreePop();
}

void RenderUI::renderLightSources()
{
    if (!ImGui::TreeNode("Point Light Sources"))
        return;

    ImGui::BeginTable("PointLightsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    // Table header
    ImGui::TableSetupColumn("Point Light Index");
    ImGui::TableSetupColumn("Linear");
    ImGui::TableSetupColumn("Quadratic");
    ImGui::TableHeadersRow();

    // Checkbox and input fields
    for (int i = 0; i < m_renderManager->m_pointLights.size(); i++)
    {
        ImGui::TableNextRow();

        LightSource &light = m_renderManager->m_pointLights[i];

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("m_pointLights: %d", i);

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat(("##Linear" + std::to_string(i)).c_str(), &light.linear);

        ImGui::TableSetColumnIndex(2);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat(("##Quadratic" + std::to_string(i)).c_str(), &light.quadratic);
    }

    ImGui::EndTable();

    ImGui::TreePop();
}

void RenderUI::renderDebug()
{
    if (!ImGui::TreeNode("Debug##RenderUI::renderDebug"))
        return;

    int totalSourceCount = m_renderManager->m_pbrSources.size();
    int visibleSourceCount = m_renderManager->m_visiblePbrSources.size() + m_renderManager->m_visiblePbrAnimSources.size();
    ImGui::Text("total source count: %d", totalSourceCount);
    ImGui::Text("visible source count: %d", visibleSourceCount);
    ImGui::Checkbox("m_debugCulling", &m_renderManager->m_debugCulling);
    ImGui::Checkbox("m_cullFront", &m_renderManager->m_cullFront);
    if (ImGui::Checkbox("m_drawCullingAabb", &m_renderManager->m_drawCullingAabb))
    {
        m_renderManager->m_cullingManager->m_debugDrawer->setDebugMode(m_renderManager->m_drawCullingAabb ? btIDebugDraw::DBG_DrawWireframe
                                                                                                          : btIDebugDraw::DBG_NoDebug);
    }
    ImGui::Checkbox("m_lightAreaDebug", &m_renderManager->m_lightAreaDebug);
    ImGui::Checkbox("m_lightSurfaceDebug", &m_renderManager->m_lightSurfaceDebug);
    //
    VectorUI::renderVec3("m_shadowBias", m_renderManager->m_shadowBias, 0.001f);

    ImGui::TreePop();
}

void RenderUI::renderGBuffer()
{
    if (!ImGui::TreeNode("G-Buffer##RenderUI::renderGBuffer"))
        return;

    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);
    float width = m_renderManager->m_gBuffer->m_width;
    float height = m_renderManager->m_gBuffer->m_height;
    float aspectRatio = width / height;
    float desiredWidth = 200.0f;
    float desiredHeight = desiredWidth / aspectRatio;

    ImVec2 size(desiredWidth, desiredHeight);

    // Begin the table
    ImGui::BeginTable("G-Buffer-Table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

    // Table header
    ImGui::TableSetupColumn("G-Buffer Position");
    ImGui::TableSetupColumn("G-Buffer Normal");
    ImGui::TableSetupColumn("G-Buffer Albedo");
    ImGui::TableSetupColumn("G-Buffer AO-Rough-Metal");
    ImGui::TableHeadersRow();

    // Image and label cells
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gPosition)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gNormalShadow)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(2);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gAlbedo)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(3);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gAoRoughMetal)), size, uv0, uv1);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gViewPosition)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_gBuffer->m_gViewNormal)), size, uv0, uv1);

    // End the table
    ImGui::EndTable();

    ImGui::TreePop();
}

void RenderUI::renderSSAO()
{
    if (!ImGui::TreeNode("SSAO##RenderUI::renderSSAO"))
        return;

    renderSSAOTextures();
    // ImGui::DragInt("noiseSize", &m_renderManager->m_ssao->noiseSize, 1);
    ImGui::DragInt("kernelSize", &m_renderManager->m_ssao->kernelSize, 1);
    ImGui::DragFloat("radius", &m_renderManager->m_ssao->radius, 0.001f);
    ImGui::DragFloat("bias", &m_renderManager->m_ssao->bias, 0.001f);
    ImGui::DragFloat("strength", &m_renderManager->m_ssao->strength, 0.001f);

    ImGui::TreePop();
}

void RenderUI::renderSSAOTextures()
{
    if (!ImGui::TreeNode("SSAO Textures##RenderUI::renderSSAOTextures"))
        return;

    ImVec2 uv0(0.0f, 1.0f);
    ImVec2 uv1(1.0f, 0.0f);
    float width = m_renderManager->m_gBuffer->m_width;
    float height = m_renderManager->m_gBuffer->m_height;
    float aspectRatio = width / height;
    float desiredWidth = 400.0f;
    float desiredHeight = desiredWidth / aspectRatio;
    ImVec2 size(desiredWidth, desiredHeight);

    ImGui::BeginTable("SSAO-Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
    ImGui::TableSetupColumn("SSAO Color");
    ImGui::TableSetupColumn("SSAO Blur");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_ssao->ssaoColorBuffer)), size, uv0, uv1);

    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<void *>(static_cast<uintptr_t>(m_renderManager->m_ssao->ssaoColorBufferBlur)), size, uv0, uv1);

    // End the table
    ImGui::EndTable();

    ImGui::TreePop();
}

void RenderUI::renderPostProcess()
{
    if (!ImGui::TreeNode("Post Process##RenderUI::renderPostProcess"))
        return;

    renderToneMapping();
    renderBloom();
    ImGui::DragFloat("m_exposure", &m_renderManager->m_postProcess->m_exposure, 0.001f);
    ImGui::DragFloat("m_contrastBright", &m_renderManager->m_postProcess->m_contrastBright, 0.01f);
    ImGui::DragFloat("m_contrastDark", &m_renderManager->m_postProcess->m_contrastDark, 0.01f);
    //
    ImGui::DragFloat("fogMaxDist", &m_renderManager->fogMaxDist, 100.0f);
    ImGui::DragFloat("fogMinDist", &m_renderManager->fogMinDist, 100.0f);
    ImGui::ColorEdit4("fogColor", &m_renderManager->fogColor[0]);
    // TODO: directional light source
    if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        VectorUI::renderVec3("m_sunColor", m_renderManager->m_sunColor, 1.f);
        ImGui::DragFloat("m_sunIntensity", &m_renderManager->m_sunIntensity, 0.1f);
    }

    ImGui::TreePop();
}

void RenderUI::renderBloom()
{
    if (!ImGui::TreeNode("Bloom##RenderUI::renderBloom"))
        return;

    ImGui::Checkbox("m_karisAverageOnDownsample", &m_renderManager->m_bloomManager->m_karisAverageOnDownsample);
    // ImGui::DragFloat("m_threshold", &m_renderManager->m_bloomManager->m_threshold, 0.01f);
    // ImGui::DragFloat("m_softThreshold", &m_renderManager->m_bloomManager->m_softThreshold, 0.01f);
    ImGui::DragFloat("m_filterRadius", &m_renderManager->m_bloomManager->m_filterRadius, 0.001f);
    ImGui::DragFloat("m_bloomIntensity", &m_renderManager->m_postProcess->m_bloomIntensity, 0.01f);

    ImGui::TreePop();
}

void RenderUI::renderToneMapping()
{
    if (!ImGui::TreeNode("Tone Mapping##RenderUI::renderToneMapping"))
        return;

    ImGui::DragFloat("m_A", &m_renderManager->m_postProcess->m_A, 0.001f);
    ImGui::DragFloat("m_B", &m_renderManager->m_postProcess->m_B, 0.001f);
    ImGui::DragFloat("m_C", &m_renderManager->m_postProcess->m_C, 0.001f);
    ImGui::DragFloat("m_D", &m_renderManager->m_postProcess->m_D, 0.001f);
    ImGui::DragFloat("m_E", &m_renderManager->m_postProcess->m_E, 0.001f);
    ImGui::DragFloat("m_F", &m_renderManager->m_postProcess->m_F, 0.001f);
    ImGui::DragFloat("m_W", &m_renderManager->m_postProcess->m_W, 0.001f);

    ImGui::TreePop();
}
