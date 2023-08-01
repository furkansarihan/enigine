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

    // Post process
    m_postProcess = new PostProcess(1.f, 1.f);
}

RenderManager::~RenderManager()
{
    delete m_pbrManager;

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
        m_linkSources[i]->transform.setModelMatrix(m_linkSources[i]->transformLink->getModelMatrix());
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
}

void RenderManager::renderDepth()
{
    // setup shadowmap
    m_shadowManager->setup((float)m_screenW, (float)m_screenH);
    glm::mat4 depthViewMatrix = m_shadowManager->getDepthViewMatrix();
    m_inverseDepthViewMatrix = glm::inverse(depthViewMatrix);

    // render depth
    m_shadowmapManager->bindFramebuffer();
    // TODO: frustum culling per split
    for (int i = 0; i < m_shadowManager->m_splitCount; i++)
    {
        m_shadowmapManager->bindTextureArray(i);
        glm::mat4 depthP = m_shadowManager->m_depthPMatrices[i];
        glm::mat4 depthVP = depthP * depthViewMatrix;

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
            m_pbrTerrainSources[i]->terrain->drawDepth(terrainDepthShader, depthViewMatrix, depthP, nearPlaneCenter);
        for (int i = 0; i < m_basicTerrainSources.size(); i++)
            m_basicTerrainSources[i]->terrain->drawDepth(terrainDepthShader, depthViewMatrix, depthP, nearPlaneCenter);

        // Draw objects
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        if (m_cullFront)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);

        // draw each object
        for (int i = 0; i < m_pbrSources.size(); i++)
        {
            RenderSource *source = m_pbrSources[i];

            depthShader.use();
            depthShader.setMat4("MVP", depthVP * source->transform.getModelMatrix());
            source->model->draw(depthShader, true);
        }
        for (int i = 0; i < m_basicSources.size(); i++)
        {
            RenderSource *source = m_basicSources[i];

            if (source->animator)
            {
                animDepthShader.use();
                animDepthShader.setMat4("projection", depthP);
                animDepthShader.setMat4("view", depthViewMatrix);

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

    // render scene
    m_frustumDistances = m_shadowManager->getFrustumDistances();

    // draw each terrain
    // TODO: m_basicTerrainSources
    // TODO: opaque render source - own draw function
    for (int i = 0; i < m_pbrTerrainSources.size(); i++)
    {
        RenderTerrainSource *source = m_pbrTerrainSources[i];

        // terrain
        glm::mat4 cullProjection = m_projection;
        glm::mat4 cullView = m_view;
        glm::vec3 cullViewPos = m_camera->position;

        // TODO: move
        // if (terrainUI.m_debugCulling)
        // {
        //     cullProjection = debugCamera.getProjectionMatrix(m_screenW, m_screenH);
        //     cullView = debugCamera.getViewMatrix();
        //     cullViewPos = debugCamera.position;
        // }
        // else
        // {
        //     debugCamera.position = m_camera->position;
        //     debugCamera.front = m_camera->front;
        //     debugCamera.right = m_camera->right;
        //     debugCamera.up = m_camera->up;
        // }

        source->terrain->drawColor(m_pbrManager, terrainPBRShader, m_shadowManager->m_lightPos,
                                   m_sunColor * m_sunIntensity, m_lightPower,
                                   m_view, m_projection, m_camera->position,
                                   cullView, cullProjection, cullViewPos,
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
    for (int i = 0; i < m_pbrSources.size(); i++)
    {
        RenderSource *source = m_pbrSources[i];
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
    for (int i = 0; i < m_basicSources.size(); i++)
    {
        RenderSource *source = m_basicSources[i];

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
    for (int i = 0; i < m_pbrSources.size(); i++)
    {
        RenderSource *source = m_pbrSources[i];
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

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(postProcessShader.id, "renderedTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);

    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderManager::addSource(RenderSource *source)
{
    if (source->type == ShaderType::pbr)
        m_pbrSources.push_back(source);
    else
        m_basicSources.push_back(source);

    if (source->transformLink)
        m_linkSources.push_back(source);
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
