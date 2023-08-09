#include "render_manager.h"

RenderManager::RenderManager(ShaderManager *shaderManager, Camera *camera, Model *cube, Model *quad, unsigned int quad_vao)
    : m_shaderManager(shaderManager),
      m_camera(camera),
      cube(cube),
      quad(quad),
      quad_vao(quad_vao)
{
    shaderManager->addShader(ShaderDynamic(&pbrShader, "../src/assets/shaders/pbr.vs", "../src/assets/shaders/pbr.fs"));
    shaderManager->addShader(ShaderDynamic(&animShader, "../src/assets/shaders/anim.vs", "../src/assets/shaders/anim.fs"));
    shaderManager->addShader(ShaderDynamic(&depthShader, "../src/assets/shaders/simple-shader.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&animDepthShader, "../src/assets/shaders/anim.vs", "../src/assets/shaders/depth-shader.fs"));
    // terrain
    shaderManager->addShader(ShaderDynamic(&terrainPBRShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-pbr.fs"));
    shaderManager->addShader(ShaderDynamic(&terrainBasicShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&terrainDepthShader, "../src/assets/shaders/terrain-shadow.vs", "../src/assets/shaders/depth-shader.fs"));

    shaderManager->addShader(ShaderDynamic(&cubemapShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/cubemap.fs"));
    shaderManager->addShader(ShaderDynamic(&postProcessShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/post-process.fs"));

    // PBR Shaders
    shaderManager->addShader(ShaderDynamic(&hdrToCubemapShader, "../src/assets/shaders/hdr-to-cubemap.vs", "../src/assets/shaders/hdr-to-cubemap.fs"));
    shaderManager->addShader(ShaderDynamic(&irradianceShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/irradiance.fs"));
    shaderManager->addShader(ShaderDynamic(&prefilterShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/prefilter.fs"));
    shaderManager->addShader(ShaderDynamic(&brdfShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/brdf.fs"));

    m_debugCamera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // PBR Setup
    m_pbrManager = new PbrManager();
    // TODO: variable skybox
    // TODO: cache the textures?
    m_pbrManager->setupCubemap(cube, hdrToCubemapShader);
    m_pbrManager->setupIrradianceMap(cube, irradianceShader);
    m_pbrManager->setupPrefilterMap(cube, prefilterShader);
    m_pbrManager->setupBrdfLUTTexture(quad_vao, brdfShader);

    // Shadowmap setup
    // TODO: why works without ids?
    std::vector<unsigned int> shaderIds;
    shaderIds.push_back(pbrShader.id);
    shaderIds.push_back(animShader.id);
    shaderIds.push_back(terrainPBRShader.id);
    shaderIds.push_back(terrainBasicShader.id);

    m_shadowManager = new ShadowManager(m_camera, shaderIds);
    m_shadowmapManager = new ShadowmapManager(m_shadowManager->m_splitCount, 1024);

    m_cullingManager = new CullingManager();
    m_postProcess = new PostProcess(1.f, 1.f);
}

RenderManager::~RenderManager()
{
    delete m_pbrManager;
    delete m_debugCamera;

    for (int i = 0; i < m_pbrSources.size(); i++)
    {
        if (m_pbrSources[i]->transformLink)
            delete m_pbrSources[i]->transformLink;
        delete m_pbrSources[i];
    }

    for (int i = 0; i < m_basicSources.size(); i++)
    {
        if (m_basicSources[i]->transformLink)
            delete m_basicSources[i]->transformLink;
        delete m_basicSources[i];
    }

    for (int i = 0; i < m_particleSources.size(); i++)
    {
        delete m_particleSources[i];
    }
}

void RenderManager::updateTransforms()
{
    // TODO: better way?
    for (int i = 0; i < m_linkSources.size(); i++)
        m_linkSources[i]->setModelMatrix(m_linkSources[i]->transformLink->getModelMatrix());
}

void RenderManager::setupFrame(GLFWwindow *window)
{
    // clear window
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update view and projection matrix
    glfwGetFramebufferSize(window, &m_screenW, &m_screenH);
    m_projection = m_camera->getProjectionMatrix((float)m_screenW, (float)m_screenH);
    m_view = m_camera->getViewMatrix();
    m_viewProjection = m_projection * m_view;

    m_cullProjection = m_projection;
    m_cullView = m_view;
    m_cullViewProjection = m_viewProjection;
    m_cullViewPos = m_camera->position;
    if (m_debugCulling)
    {
        m_cullProjection = m_debugCamera->getProjectionMatrix((float)m_screenW, (float)m_screenH);
        m_cullView = m_debugCamera->getViewMatrix();
        m_cullViewProjection = m_cullProjection * m_cullView;
        m_cullViewPos = m_debugCamera->position;
    }
    else
    {
        m_debugCamera->position = m_camera->position;
        m_debugCamera->front = m_camera->front;
        m_debugCamera->right = m_camera->right;
        m_debugCamera->up = m_camera->up;
    }

    // setup shadowmap
    if (m_debugCulling)
        m_shadowManager->m_camera = m_debugCamera;
    else
        m_shadowManager->m_camera = m_camera;
    m_shadowManager->setupFrustum((float)m_screenW, (float)m_screenH, m_cullProjection);
    m_depthViewMatrix = m_shadowManager->getDepthViewMatrix();
    m_inverseDepthViewMatrix = glm::inverse(m_depthViewMatrix);

    // view frustum culling
    m_cullingManager->setupFrame(m_cullViewProjection);
    m_visiblePbrSources.clear();
    m_visibleBasicSources.clear();

    std::vector<CulledObject> objects = m_cullingManager->getObjects(m_shadowManager->m_aabb.min,
                                                                     m_shadowManager->m_aabb.max,
                                                                     m_cullViewPos);
    std::vector<aabb> objectAabbs;

    for (int i = 0; i < objects.size(); i++)
    {
        CulledObject &object = objects[i];
        RenderSource *source = static_cast<RenderSource *>(object.userPointer);

        source->cullIndex = i;
        objectAabbs.push_back(aabb(object.aabbMin, object.aabbMax));

        if (source->type == ShaderType::pbr)
            m_visiblePbrSources.push_back(source);
        else
            m_visibleBasicSources.push_back(source);
    }

    m_shadowManager->setupLightAabb(objectAabbs);

    // TODO: variable size
    std::vector<float> &frustumDistances = m_shadowManager->m_frustumDistances;
    for (int i = 0; i < frustumDistances.size(); i++)
    {
        m_frustumDistances[i] = frustumDistances[i];
    }
}

void RenderManager::renderDepth()
{
    m_shadowmapManager->bindFramebuffer();
    // TODO: frustum culling per split
    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        int frustumIndex = i;
        m_shadowmapManager->bindTextureArray(i);
        glm::mat4 depthP = m_shadowManager->m_depthPMatrices[i];
        glm::mat4 depthVP = depthP * m_depthViewMatrix;

        // Draw each terrain
        glm::vec3 nearPlaneEdges[4];
        for (int j = 0; j < 4; j++)
        {
            glm::vec4 worldPoint = m_inverseDepthViewMatrix * glm::vec4(m_shadowManager->m_frustums[i].lightAABB[j], 1.0f);
            glm::vec3 worldPoint3 = glm::vec3(worldPoint) / worldPoint.w;
            nearPlaneEdges[j] = worldPoint3;
        }
        glm::vec3 nearPlaneCenter = (nearPlaneEdges[0] + nearPlaneEdges[1] + nearPlaneEdges[2] + nearPlaneEdges[3]) / 4.0f;

        // TODO: terrain only cascade - covers all area - last one
        for (int i = 0; i < m_pbrTerrainSources.size(); i++)
            m_pbrTerrainSources[i]->terrain->drawDepth(terrainDepthShader, m_depthViewMatrix, depthP, nearPlaneCenter);
        for (int i = 0; i < m_basicTerrainSources.size(); i++)
            m_basicTerrainSources[i]->terrain->drawDepth(terrainDepthShader, m_depthViewMatrix, depthP, nearPlaneCenter);

        // Draw objects
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        if (m_cullFront)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);

        // draw each object
        for (int i = 0; i < m_visiblePbrSources.size(); i++)
        {
            RenderSource *source = m_visiblePbrSources[i];

            if (!inShadowFrustum(source, frustumIndex))
                continue;

            depthShader.use();
            depthShader.setMat4("MVP", depthVP * source->transform.getModelMatrix());
            source->model->draw(depthShader, true);
        }
        for (int i = 0; i < m_visibleBasicSources.size(); i++)
        {
            RenderSource *source = m_visibleBasicSources[i];

            if (!inShadowFrustum(source, frustumIndex))
                continue;

            if (source->animator)
            {
                animDepthShader.use();
                animDepthShader.setMat4("projection", depthP);
                animDepthShader.setMat4("view", m_depthViewMatrix);

                // TODO: set as block
                auto transforms = source->animator->m_finalBoneMatrices;
                for (int i = 0; i < transforms.size(); ++i)
                    animDepthShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

                animDepthShader.setMat4("model", source->transform.getModelMatrix());
                source->model->draw(animDepthShader, true);
            }
            else
            {
                depthShader.use();
                depthShader.setMat4("MVP", depthVP * source->transform.getModelMatrix());
                source->model->draw(depthShader, true);
            }
        }

        glDisable(GL_CULL_FACE);
    }
}

void RenderManager::renderOpaque()
{
    // render to post process texture
    m_postProcess->updateFramebuffer((float)m_screenW, (float)m_screenH);
    glBindFramebuffer(GL_FRAMEBUFFER, m_postProcess->m_framebufferObject);
    glViewport(0, 0, m_screenW, m_screenH);
    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw skybox
    glDepthMask(GL_FALSE);
    cubemapShader.use();
    cubemapShader.setMat4("projection", m_projection);
    cubemapShader.setMat4("view", m_view);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(cubemapShader.id, "environmentMap"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->m_skyboxTexture);
    cube->draw(cubemapShader);
    glDepthMask(GL_TRUE);

    // draw each terrain
    // TODO: m_basicTerrainSources
    // TODO: opaque render source - own draw function
    for (int i = 0; i < m_pbrTerrainSources.size(); i++)
    {
        RenderTerrainSource *source = m_pbrTerrainSources[i];

        source->terrain->drawColor(m_pbrManager, terrainPBRShader, m_shadowManager->m_lightPos,
                                   m_sunColor * m_sunIntensity, m_lightPower,
                                   m_view, m_projection, m_camera->position,
                                   m_cullView, m_cullProjection, m_cullViewPos,
                                   m_shadowmapManager->m_textureArray,
                                   m_camera->position, m_camera->front,
                                   m_frustumDistances, m_shadowBias,
                                   m_camera->projectionMode == ProjectionMode::Ortho);
    }

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // setup pbr shader
    pbrShader.use();
    pbrShader.setMat4("view", m_view);
    pbrShader.setMat4("projection", m_projection);
    pbrShader.setVec3("camPos", m_camera->position);
    pbrShader.setVec3("lightDirection", m_shadowManager->m_lightPos);
    pbrShader.setVec3("lightColor", m_sunColor * m_sunIntensity);
    pbrShader.setVec3("CamView", m_shadowManager->m_camera->front);
    pbrShader.setVec4("FrustumDistances", m_frustumDistances);
    pbrShader.setVec3("Bias", m_shadowBias);

    // TODO: light positions
    // for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    // {
    //     pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
    //     pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    // }

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrShader.id, "irradianceMap"), 8);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->irradianceMap);

    glActiveTexture(GL_TEXTURE0 + 9);
    glUniform1i(glGetUniformLocation(pbrShader.id, "prefilterMap"), 9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->prefilterMap);

    glActiveTexture(GL_TEXTURE0 + 10);
    glUniform1i(glGetUniformLocation(pbrShader.id, "brdfLUT"), 10);
    glBindTexture(GL_TEXTURE_2D, m_pbrManager->brdfLUTTexture);

    glActiveTexture(GL_TEXTURE0 + 11);
    glUniform1i(glGetUniformLocation(pbrShader.id, "ShadowMap"), 11);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    // draw each pbr
    for (int i = 0; i < m_visiblePbrSources.size(); i++)
    {
        RenderSource *source = m_visiblePbrSources[i];
        pbrShader.setBool("mergedPBRTextures", source->mergedPBRTextures);
        pbrShader.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(pbrShader, true);
    }

    // setup basic shader
    animShader.use();
    animShader.setMat4("projection", m_projection);
    animShader.setMat4("view", m_view);
    animShader.setVec3("lightDir", m_shadowManager->m_lightPos);
    // TODO: light
    animShader.setVec3("lightColor", glm::normalize(m_sunColor));
    animShader.setFloat("lightPower", m_sunIntensity);
    animShader.setVec3("camPos", m_camera->position);
    animShader.setVec3("CamView", m_shadowManager->m_camera->front);
    animShader.setVec4("FrustumDistances", m_frustumDistances);
    animShader.setVec3("Bias", m_shadowBias);

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(animShader.id, "ShadowMap"), 8);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    // render each basic
    // TODO: basic without anim
    for (int i = 0; i < m_visibleBasicSources.size(); i++)
    {
        RenderSource *source = m_visibleBasicSources[i];

        if (source->animator)
        {
            // TODO: set as block
            auto transforms = source->animator->m_finalBoneMatrices;
            for (int i = 0; i < transforms.size(); ++i)
                animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        animShader.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(animShader, true);
    }

    glDisable(GL_CULL_FACE);
}

void RenderManager::renderBlend()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // render particle engines
    for (int i = 0; i < m_particleSources.size(); i++)
    {
        if (m_particleSources[i]->transformLink)
        {
            // TODO: refactor
            glm::mat4 model = m_particleSources[i]->transformLink->getModelMatrix();
            m_particleSources[i]->particleEngine->m_position = CommonUtil::positionFromModel(model);
            m_particleSources[i]->particleEngine->m_direction = glm::normalize(glm::mat3(model) * glm::vec3(0.f, 0.f, 1.f));
        }

        m_particleSources[i]->particleEngine->drawParticles(m_particleSources[i]->shader, quad, m_viewProjection);
    }

    glDisable(GL_BLEND);
}

void RenderManager::renderTransmission()
{
    // create mipmap
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_screenW, m_screenH);

    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindFramebuffer(GL_FRAMEBUFFER, m_postProcess->m_framebufferObject);
    glViewport(0, 0, m_screenW, m_screenH);

    // render transmission
    // TODO: reuse setup pbr
    pbrShader.use();
    pbrShader.setMat4("view", m_view);
    pbrShader.setMat4("projection", m_projection);
    pbrShader.setVec3("camPos", m_camera->position);
    pbrShader.setVec3("lightDirection", m_shadowManager->m_lightPos);
    pbrShader.setVec3("lightColor", m_sunColor * m_sunIntensity);
    pbrShader.setVec3("CamView", m_shadowManager->m_camera->front);
    pbrShader.setVec4("FrustumDistances", m_frustumDistances);
    pbrShader.setVec3("Bias", m_shadowBias);
    pbrShader.setVec2("u_TransmissionFramebufferSize", glm::vec2(m_screenW, m_screenH));

    // for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
    // {
    //     pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
    //     pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
    // }

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrShader.id, "irradianceMap"), 8);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->irradianceMap);

    glActiveTexture(GL_TEXTURE0 + 9);
    glUniform1i(glGetUniformLocation(pbrShader.id, "prefilterMap"), 9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->prefilterMap);

    glActiveTexture(GL_TEXTURE0 + 10);
    glUniform1i(glGetUniformLocation(pbrShader.id, "brdfLUT"), 10);
    glBindTexture(GL_TEXTURE_2D, m_pbrManager->brdfLUTTexture);

    glActiveTexture(GL_TEXTURE0 + 11);
    glUniform1i(glGetUniformLocation(pbrShader.id, "ShadowMap"), 11);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    glActiveTexture(GL_TEXTURE0 + 12);
    glUniform1i(glGetUniformLocation(pbrShader.id, "u_TransmissionFramebufferSampler"), 12);
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);

    // render each transmission mesh
    for (int i = 0; i < m_visiblePbrSources.size(); i++)
    {
        RenderSource *source = m_visiblePbrSources[i];
        pbrShader.setBool("mergedPBRTextures", source->mergedPBRTextures);
        pbrShader.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(pbrShader, false);
    }
}

void RenderManager::renderPostProcess()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_screenW, m_screenH);

    // remove mipmaps
    // TODO: only if mipmaps created
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    postProcessShader.use();
    postProcessShader.setVec2("screenSize", glm::vec2((float)m_screenW, (float)m_screenH));
    // postProcessShader.setFloat("blurOffset", blurOffset);
    postProcessShader.setFloat("exposure", m_postProcess->m_exposure);
    postProcessShader.setFloat("contrastBright", m_postProcess->m_contrastBright);
    postProcessShader.setFloat("contrastDark", m_postProcess->m_contrastDark);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(postProcessShader.id, "renderedTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);

    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderManager::addSource(RenderSource *source)
{
    if (!source->model)
    {
        std::cout << "RenderManager:addSource: missing model object" << std::endl;
        return;
    }

    source->m_renderManager = this;

    if (source->type == ShaderType::pbr)
        m_pbrSources.push_back(source);
    else
        m_basicSources.push_back(source);

    if (source->transformLink)
        m_linkSources.push_back(source);

    glm::vec3 size = (source->model->aabbMax - source->model->aabbMin) / 2.0f;
    m_cullingManager->addObject(source, size, source->transform.getModelMatrix());
}

RenderTerrainSource *RenderManager::addTerrainSource(ShaderType type, eTransform transform, Terrain *terrain)
{
    RenderTerrainSource *source = new RenderTerrainSource(type, transform, terrain);

    if (type == ShaderType::pbr)
        m_pbrTerrainSources.push_back(source);
    else
        m_basicTerrainSources.push_back(source);

    return source;
}

void RenderManager::addParticleSource(RenderParticleSource *source)
{
    m_particleSources.push_back(source);
}

bool RenderManager::inShadowFrustum(RenderSource *source, int frustumIndex)
{
    if (source->cullIndex == -1)
        return false;

    const auto &frustumIndexes = m_shadowManager->m_sceneObjects[source->cullIndex].frustumIndexes;
    if (std::find(frustumIndexes.begin(), frustumIndexes.end(), frustumIndex) == frustumIndexes.end())
        return false;

    return true;
}

// RenderSource

void RenderSource::setTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
    transform.setTransform(position, rotation, scale);

    if (!m_renderManager->m_cullingManager)
        return;

    m_renderManager->m_cullingManager->updateObject(this, transform.getModelMatrix());
}

void RenderSource::setModelMatrix(glm::mat4 modelMatrix)
{
    transform.setModelMatrix(modelMatrix);

    if (!m_renderManager->m_cullingManager)
        return;

    m_renderManager->m_cullingManager->updateObject(this, modelMatrix);
}
