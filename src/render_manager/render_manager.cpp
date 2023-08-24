#include "render_manager.h"

RenderManager::RenderManager(ShaderManager *shaderManager, Camera *camera, Model *cube, Model *quad, Model *sphere, unsigned int quad_vao)
    : m_shaderManager(shaderManager),
      m_camera(camera),
      cube(cube),
      quad(quad),
      sphere(sphere),
      quad_vao(quad_vao)
{
    shaderManager->addShader(ShaderDynamic(&pbrDeferredPre, "../src/assets/shaders/pbr.vs", "../src/assets/shaders/pbr-deferred-pre.fs"));
    shaderManager->addShader(ShaderDynamic(&pbrDeferredPreAnim, "../src/assets/shaders/anim.vs", "../src/assets/shaders/pbr-deferred-pre.fs"));
    shaderManager->addShader(ShaderDynamic(&pbrDeferredAfter, "../src/assets/shaders/pbr-deferred-after.vs", "../src/assets/shaders/pbr-deferred-after.fs"));
    shaderManager->addShader(ShaderDynamic(&pbrDeferredPointLight, "../src/assets/shaders/pbr-deferred-point-light.vs", "../src/assets/shaders/pbr-deferred-point-light.fs"));
    shaderManager->addShader(ShaderDynamic(&pbrTransmission, "../src/assets/shaders/pbr.vs", "../src/assets/shaders/pbr.fs"));

    shaderManager->addShader(ShaderDynamic(&depthShader, "../src/assets/shaders/simple-shader.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&depthShaderAnim, "../src/assets/shaders/anim.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&lightVolume, "../src/assets/shaders/light-volume.vs", "../src/assets/shaders/light-volume.fs"));
    shaderManager->addShader(ShaderDynamic(&lightVolumeDebug, "../src/assets/shaders/light-volume.vs", "../src/assets/shaders/simple-shader.fs"));

    shaderManager->addShader(ShaderDynamic(&shaderSSAO, "../src/assets/shaders/ssao.vs", "../src/assets/shaders/ssao.fs"));
    shaderManager->addShader(ShaderDynamic(&shaderSSAOBlur, "../src/assets/shaders/ssao.vs", "../src/assets/shaders/ssao-blur.fs"));

    // terrain
    shaderManager->addShader(ShaderDynamic(&terrainPBRShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-pbr-deferred-pre.fs"));
    shaderManager->addShader(ShaderDynamic(&terrainBasicShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-shader.fs"));
    shaderManager->addShader(ShaderDynamic(&terrainDepthShader, "../src/assets/shaders/terrain-shadow.vs", "../src/assets/shaders/depth-shader.fs"));

    shaderManager->addShader(ShaderDynamic(&skyboxShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/skybox.fs"));
    shaderManager->addShader(ShaderDynamic(&postProcessShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/post-process.fs"));

    // PBR Shaders
    shaderManager->addShader(ShaderDynamic(&hdrToCubemapShader, "../src/assets/shaders/hdr-to-cubemap.vs", "../src/assets/shaders/hdr-to-cubemap.fs"));
    shaderManager->addShader(ShaderDynamic(&irradianceShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/irradiance.fs"));
    shaderManager->addShader(ShaderDynamic(&prefilterShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/prefilter.fs"));
    shaderManager->addShader(ShaderDynamic(&brdfShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/brdf.fs"));

    shaderManager->addShader(ShaderDynamic(&downsampleShader, "../src/assets/shaders/sample.vs", "../src/assets/shaders/downsample.fs"));
    shaderManager->addShader(ShaderDynamic(&upsampleShader, "../src/assets/shaders/sample.vs", "../src/assets/shaders/upsample.fs"));

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
    shaderIds.push_back(pbrDeferredPre.id);
    shaderIds.push_back(pbrDeferredPreAnim.id);
    shaderIds.push_back(terrainPBRShader.id);
    shaderIds.push_back(terrainBasicShader.id);

    m_shadowManager = new ShadowManager(m_camera, shaderIds);
    m_shadowmapManager = new ShadowmapManager(m_shadowManager->m_splitCount, 1024);

    m_cullingManager = new CullingManager();
    m_gBuffer = new GBuffer(1, 1);
    m_ssao = new SSAO(1, 1);
    m_postProcess = new PostProcess(1, 1);
    m_bloomManager = new BloomManager(&downsampleShader, &upsampleShader, quad_vao);

    setupLights();
}

RenderManager::~RenderManager()
{
    delete m_pbrManager;
    delete m_debugCamera;
    delete m_postProcess;
    delete m_bloomManager;

    for (int i = 0; i < m_pbrSources.size(); i++)
    {
        if (m_pbrSources[i]->transformLink)
            delete m_pbrSources[i]->transformLink;
        delete m_pbrSources[i];
    }

    for (int i = 0; i < m_particleSources.size(); i++)
    {
        delete m_particleSources[i];
    }
}

void RenderManager::setupLights()
{
    Model *lightVolume = sphere;

    glGenBuffers(1, &m_lightArrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_lightArrayBuffer);
    // TODO: ?
    glBufferData(GL_ARRAY_BUFFER, m_pointLights.size() * sizeof(LightInstance), m_lightBufferList.data(), GL_STATIC_DRAW);

    for (unsigned int i = 0; i < lightVolume->meshes.size(); i++)
    {
        unsigned int VAO = lightVolume->meshes[i]->VAO;
        glBindVertexArray(VAO);

        float size = sizeof(LightInstance);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, size, (void *)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, size, (void *)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, size, (void *)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, size, (void *)(3 * sizeof(glm::vec4)));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, size, (void *)offsetof(LightInstance, lightColor));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(LightInstance, radius));

        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(LightInstance, linear));

        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(LightInstance, quadratic));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
        glVertexAttribDivisor(9, 1);
        glVertexAttribDivisor(10, 1);

        glBindVertexArray(0);
    }
}

void RenderManager::updateTransforms()
{
    // TODO: better way?
    for (int i = 0; i < m_linkSources.size(); i++)
        m_linkSources[i]->setModelMatrix(m_linkSources[i]->transformLink->getModelMatrix());
}

void RenderManager::addLight(LightSource light)
{
    m_pointLights.push_back(light);
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
    m_visiblePbrAnimSources.clear();

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

        if (source->animator)
            m_visiblePbrAnimSources.push_back(source);
        else
            m_visiblePbrSources.push_back(source);
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
        for (int i = 0; i < m_visiblePbrAnimSources.size(); i++)
        {
            RenderSource *source = m_visiblePbrAnimSources[i];

            if (!inShadowFrustum(source, frustumIndex))
                continue;

            if (source->animator)
            {
                depthShaderAnim.use();
                depthShaderAnim.setMat4("projection", depthP);
                depthShaderAnim.setMat4("view", m_depthViewMatrix);

                // TODO: set as block
                auto transforms = source->animator->m_finalBoneMatrices;
                for (int i = 0; i < transforms.size(); ++i)
                    depthShaderAnim.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

                depthShaderAnim.setMat4("model", source->transform.getModelMatrix());
                source->model->draw(depthShaderAnim, true);
            }
        }

        glDisable(GL_CULL_FACE);
    }
}

void RenderManager::renderOpaque()
{
    // render to post process texture
    m_gBuffer->updateResolution(m_screenW, m_screenH);
    m_ssao->updateResolution(m_screenW, m_screenH);
    m_postProcess->updateResolution(m_screenW, m_screenH);
    m_bloomManager->updateResolution(m_screenW, m_screenH);

    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer->m_fbo);
    glViewport(0, 0, m_screenW, m_screenH);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    pbrDeferredPre.use();
    pbrDeferredPre.setMat4("view", m_view);
    pbrDeferredPre.setMat4("projection", m_projection);
    pbrDeferredPre.setVec3("lightDirection", m_shadowManager->m_lightPos);
    pbrDeferredPre.setVec4("FrustumDistances", m_frustumDistances);
    pbrDeferredPre.setVec3("Bias", m_shadowBias);

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrDeferredPre.id, "ShadowMap"), 8);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    // draw each pbr
    for (int i = 0; i < m_visiblePbrSources.size(); i++)
    {
        RenderSource *source = m_visiblePbrSources[i];
        pbrDeferredPre.setBool("material.aoRoughMetalMap", source->aoRoughMetalMap);
        pbrDeferredPre.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(pbrDeferredPre, true);
    }

    // setup pbr anim shader
    pbrDeferredPreAnim.use();
    pbrDeferredPreAnim.setMat4("view", m_view);
    pbrDeferredPreAnim.setMat4("projection", m_projection);
    pbrDeferredPreAnim.setVec3("lightDirection", m_shadowManager->m_lightPos);
    pbrDeferredPreAnim.setVec4("FrustumDistances", m_frustumDistances);
    pbrDeferredPreAnim.setVec3("Bias", m_shadowBias);

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrDeferredPreAnim.id, "ShadowMap"), 8);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    // render each anim
    for (int i = 0; i < m_visiblePbrAnimSources.size(); i++)
    {
        RenderSource *source = m_visiblePbrAnimSources[i];

        if (source->animator)
        {
            // TODO: set as block
            auto transforms = source->animator->m_finalBoneMatrices;
            for (int i = 0; i < transforms.size(); ++i)
                pbrDeferredPreAnim.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        pbrDeferredPreAnim.setBool("material.aoRoughMetalMap", source->aoRoughMetalMap);
        pbrDeferredPreAnim.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(pbrDeferredPreAnim, true);
    }

    glDisable(GL_CULL_FACE);
}

void RenderManager::renderSSAO()
{
    // color
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssao->ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderSSAO.use();
    // Send kernel + rotation
    for (unsigned int i = 0; i < 64; ++i)
        shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", m_ssao->ssaoKernel[i]);
    shaderSSAO.setMat4("projection", m_projection);
    shaderSSAO.setMat4("view", m_view);
    shaderSSAO.setInt("kernelSize", m_ssao->kernelSize);
    shaderSSAO.setFloat("radius", m_ssao->radius);
    shaderSSAO.setFloat("bias", m_ssao->bias);
    shaderSSAO.setFloat("strength", m_ssao->strength);
    // tile noise texture over screen based on screen dimensions divided by noise size
    shaderSSAO.setVec2("noiseScale", glm::vec2(m_screenW, m_screenH) / (float)m_ssao->noiseSize);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shaderSSAO.id, "gPosition"), 0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gPosition);

    glActiveTexture(GL_TEXTURE0 + 1);
    glUniform1i(glGetUniformLocation(shaderSSAO.id, "gNormal"), 1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gNormalShadow);

    glActiveTexture(GL_TEXTURE0 + 2);
    glUniform1i(glGetUniformLocation(shaderSSAO.id, "texNoise"), 2);
    glBindTexture(GL_TEXTURE_2D, m_ssao->noiseTexture);

    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // blur
    glBindFramebuffer(GL_FRAMEBUFFER, m_ssao->ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderSSAOBlur.use();

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shaderSSAOBlur.id, "ssaoInput"), 0);
    glBindTexture(GL_TEXTURE_2D, m_ssao->ssaoColorBuffer);

    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderManager::renderDeferredShading()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_postProcess->m_framebufferObject);

    glEnable(GL_STENCIL_TEST);

    glViewport(0, 0, m_screenW, m_screenH);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save depth for skybox
    // TODO: 230 microseconds - better way?
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer->m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_postProcess->m_framebufferObject);
    glBlitFramebuffer(0, 0, m_screenW, m_screenH, 0, 0, m_screenW, m_screenH, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // TODO: not clearing?
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilMask(0xFF);
    // TODO: remove
    // fill screen with 0 with full screen quad
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_ZERO, GL_REPLACE, GL_ZERO);

    // directional light and shadow pass
    pbrDeferredAfter.use();
    pbrDeferredAfter.setMat4("view", m_view);
    pbrDeferredAfter.setVec3("camPos", m_camera->position);
    pbrDeferredAfter.setVec3("light.direction", m_shadowManager->m_lightPos);
    pbrDeferredAfter.setVec3("light.color", m_sunColor * m_sunIntensity);
    pbrDeferredAfter.setFloat("fogMaxDist", fogMaxDist);
    pbrDeferredAfter.setFloat("fogMinDist", fogMinDist);
    pbrDeferredAfter.setVec4("fogColor", fogColor);

    glActiveTexture(GL_TEXTURE0 + 0);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "gPosition"), 0);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gPosition);

    glActiveTexture(GL_TEXTURE0 + 1);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "gNormalShadow"), 1);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gNormalShadow);

    glActiveTexture(GL_TEXTURE0 + 2);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "gAlbedo"), 2);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gAlbedo);

    glActiveTexture(GL_TEXTURE0 + 3);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "gAoRoughMetal"), 3);
    glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gAoRoughMetal);

    glActiveTexture(GL_TEXTURE0 + 4);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "ssaoSampler"), 4);
    glBindTexture(GL_TEXTURE_2D, m_ssao->ssaoColorBufferBlur);

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "irradianceMap"), 8);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->irradianceMap);

    glActiveTexture(GL_TEXTURE0 + 9);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "prefilterMap"), 9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->prefilterMap);

    glActiveTexture(GL_TEXTURE0 + 10);
    glUniform1i(glGetUniformLocation(pbrDeferredAfter.id, "brdfLUT"), 10);
    glBindTexture(GL_TEXTURE_2D, m_pbrManager->brdfLUTTexture);

    glDepthMask(GL_FALSE);
    glBindVertexArray(quad_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    // draw skybox
    skyboxShader.use();
    skyboxShader.setMat4("projection", m_projection);
    skyboxShader.setMat4("view", m_view);
    skyboxShader.setVec3("sunDirection", m_shadowManager->m_lightPos);
    skyboxShader.setVec3("sunColor", m_sunColor * m_sunIntensity);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(skyboxShader.id, "environmentMap"), 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->m_skyboxTexture);
    cube->draw(skyboxShader);

    // TODO: frustum culling

    std::vector<LightSource> lightsInsideCam;
    std::vector<LightSource> lightsOutsideCam;

    for (int i = 0; i < m_pointLights.size(); i++)
    {
        LightSource &light = m_pointLights[i];
        float camDistance = glm::abs(glm::distance(m_cullViewPos, light.position));
        // TODO: padding for camera near clip
        if (camDistance < light.radius)
            lightsInsideCam.push_back(light);
        else
            lightsOutsideCam.push_back(light);
    }

    renderLightVolumes(lightsInsideCam, true);
    renderLightVolumes(lightsOutsideCam, false);
}

void RenderManager::renderLightVolumes(std::vector<LightSource> &lights, bool camInsideVolume)
{
    if (lights.size() == 0)
        return;

    updateLightBuffer(lights);

    // light mask pass
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    // glStencilOp(sfail, dpfail, dppass);
    glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilMask(0xFF);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // TODO: why can't use glCullFace?
    lightVolume.use();
    lightVolume.setMat4("projection", m_projection);
    lightVolume.setMat4("view", m_view);
    lightVolume.setMat4("model", glm::mat4(1.0f));
    sphere->drawInstanced(lightVolume, lights.size());

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // light shade pass
    glStencilFunc(GL_LESS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x00);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    if (camInsideVolume)
    {
        glCullFace(GL_FRONT);
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    if (m_lightSurfaceDebug)
    {
        lightVolumeDebug.use();
        lightVolumeDebug.setMat4("projection", m_projection);
        lightVolumeDebug.setMat4("view", m_view);
        lightVolumeDebug.setVec4("DiffuseColor", glm::vec4(1.0f, 0.0f, 1.0f, 0.1f));
        glm::mat4 model(1.0f);
        lightVolumeDebug.setMat4("model", model);
        sphere->drawInstanced(lightVolumeDebug, lights.size());
    }
    else
    {
        pbrDeferredPointLight.use();
        pbrDeferredPointLight.setMat4("projection", m_projection);
        pbrDeferredPointLight.setMat4("view", m_view);
        pbrDeferredPointLight.setVec3("camPos", m_camera->position);
        pbrDeferredPointLight.setVec2("screenSize", glm::vec2(m_screenW, m_screenH));

        glActiveTexture(GL_TEXTURE0 + 0);
        glUniform1i(glGetUniformLocation(pbrDeferredPointLight.id, "gPosition"), 0);
        glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gPosition);

        glActiveTexture(GL_TEXTURE0 + 1);
        glUniform1i(glGetUniformLocation(pbrDeferredPointLight.id, "gNormalShadow"), 1);
        glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gNormalShadow);

        glActiveTexture(GL_TEXTURE0 + 2);
        glUniform1i(glGetUniformLocation(pbrDeferredPointLight.id, "gAlbedo"), 2);
        glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gAlbedo);

        glActiveTexture(GL_TEXTURE0 + 3);
        glUniform1i(glGetUniformLocation(pbrDeferredPointLight.id, "gAoRoughMetal"), 3);
        glBindTexture(GL_TEXTURE_2D, m_gBuffer->m_gAoRoughMetal);

        sphere->drawInstanced(pbrDeferredPointLight, lights.size());
    }

    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    //
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilMask(0xFF);

    glDisable(GL_STENCIL_TEST);

    if (m_lightAreaDebug)
    {
        if (camInsideVolume)
            glCullFace(GL_FRONT);
        else
            glCullFace(GL_BACK);

        lightVolumeDebug.use();
        lightVolumeDebug.setMat4("projection", m_projection);
        lightVolumeDebug.setMat4("view", m_view);
        lightVolumeDebug.setVec4("DiffuseColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));
        glm::mat4 model(1.0f);
        lightVolumeDebug.setMat4("model", model);
        sphere->drawInstanced(lightVolumeDebug, lights.size());
    }

    glDisable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    // light points
    lightVolume.use();
    lightVolume.setMat4("projection", m_projection);
    lightVolume.setMat4("view", m_view);
    lightVolume.setVec4("DiffuseColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.01f));
    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(0.01f));
    lightVolume.setMat4("model", model);
    // TODO: bilboarded circle
    sphere->drawInstanced(lightVolume, lights.size());
}

void RenderManager::updateLightBuffer(std::vector<LightSource> &lights)
{
    m_lightBufferList.clear();

    for (int i = 0; i < lights.size(); ++i)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, lights[i].position);
        model = glm::scale(model, glm::vec3(lights[i].radius));

        LightInstance lightInstance;
        lightInstance.model = model;
        lightInstance.lightColor = lights[i].color * lights[i].intensity;
        lightInstance.radius = lights[i].radius;
        lightInstance.linear = lights[i].linear;
        lightInstance.quadratic = lights[i].quadratic;

        m_lightBufferList.push_back(lightInstance);
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_lightArrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_lightBufferList.size() * sizeof(LightInstance), m_lightBufferList.data(), GL_STATIC_DRAW);
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
    pbrTransmission.use();
    pbrTransmission.setMat4("view", m_view);
    pbrTransmission.setMat4("projection", m_projection);
    pbrTransmission.setVec3("camPos", m_camera->position);
    pbrTransmission.setVec3("lightDirection", m_shadowManager->m_lightPos);
    pbrTransmission.setVec3("lightColor", m_sunColor * m_sunIntensity);
    pbrTransmission.setVec3("CamView", m_shadowManager->m_camera->front);
    pbrTransmission.setVec4("FrustumDistances", m_frustumDistances);
    pbrTransmission.setVec3("Bias", m_shadowBias);
    pbrTransmission.setVec2("u_TransmissionFramebufferSize", glm::vec2(m_screenW, m_screenH));

    glActiveTexture(GL_TEXTURE0 + 8);
    glUniform1i(glGetUniformLocation(pbrTransmission.id, "irradianceMap"), 8);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->irradianceMap);

    glActiveTexture(GL_TEXTURE0 + 9);
    glUniform1i(glGetUniformLocation(pbrTransmission.id, "prefilterMap"), 9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->prefilterMap);

    glActiveTexture(GL_TEXTURE0 + 10);
    glUniform1i(glGetUniformLocation(pbrTransmission.id, "brdfLUT"), 10);
    glBindTexture(GL_TEXTURE_2D, m_pbrManager->brdfLUTTexture);

    glActiveTexture(GL_TEXTURE0 + 11);
    glUniform1i(glGetUniformLocation(pbrTransmission.id, "ShadowMap"), 11);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_shadowmapManager->m_textureArray);

    glActiveTexture(GL_TEXTURE0 + 12);
    glUniform1i(glGetUniformLocation(pbrTransmission.id, "u_TransmissionFramebufferSampler"), 12);
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);

    // render each transmission mesh
    for (int i = 0; i < m_visiblePbrSources.size(); i++)
    {
        RenderSource *source = m_visiblePbrSources[i];
        pbrTransmission.setBool("material.aoRoughMetalMap", source->aoRoughMetalMap);
        pbrTransmission.setMat4("model", source->transform.getModelMatrix());
        source->model->draw(pbrTransmission, false);
    }
}

void RenderManager::renderPostProcess()
{
    m_bloomManager->renderBloomTexture(m_postProcess->m_texture);

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
    postProcessShader.setFloat("bloomIntensity", m_postProcess->m_bloomIntensity);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(postProcessShader.id, "renderedTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, m_postProcess->m_texture);

    glActiveTexture(GL_TEXTURE0 + 1);
    glUniform1i(glGetUniformLocation(postProcessShader.id, "bloomTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, m_bloomManager->bloomTexture());

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

    m_pbrSources.push_back(source);

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
