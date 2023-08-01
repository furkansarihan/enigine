#ifndef render_manager_hpp
#define render_manager_hpp

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../model/model.h"
#include "../animation/animator.h"
#include "../terrain/terrain.h"
#include "../transform/transform.h"
#include "../shader_manager/shader_manager.h"
#include "../pbr_manager/pbr_manager.h"
#include "../camera/camera.h"
#include "../shadowmap_manager/shadowmap_manager.h"
#include "../shadow_manager/shadow_manager.h"
#include "../post_process/post_process.h"
#include "../transform_link/transform_link.h"
#include "../particle_engine/particle_engine.h"
#include "../utils/common.h"

enum ShaderType
{
    pbr,
    basic
};

class RenderSource
{
public:
    ShaderType type;
    eTransform transform;
    eTransform offset;
    Model *model = nullptr;
    Animator *animator = nullptr;
    TransformLink *transformLink = nullptr;
    // TODO: auto detect
    bool mergedPBRTextures = false;

    RenderSource(ShaderType type, eTransform transform, eTransform offset, Model *model, Animator *animator, TransformLink *transformLink, bool mergedPBRTextures)
        : type(type),
          transform(transform),
          offset(offset),
          model(model),
          animator(animator),
          transformLink(transformLink),
          mergedPBRTextures(mergedPBRTextures){};
};

class RenderTerrainSource
{
public:
    ShaderType type;
    eTransform transform;
    Terrain *terrain;

    RenderTerrainSource(ShaderType type, eTransform transform, Terrain *terrain)
        : type(type),
          transform(transform),
          terrain(terrain){};
};

class RenderParticleSource
{
public:
    Shader *shader;
    ParticleEngine *particleEngine;
    TransformLink *transformLink = nullptr;

    RenderParticleSource(Shader *shader, ParticleEngine *particleEngine)
        : shader(shader),
          particleEngine(particleEngine){};

    RenderParticleSource(Shader *shader, ParticleEngine *particleEngine, TransformLink *transformLink)
        : shader(shader),
          particleEngine(particleEngine),
          transformLink(transformLink){};
};

class RenderSourceBuilder
{
public:
    RenderSourceBuilder(ShaderType type) : m_type(type) {}

    RenderSourceBuilder &setTransform(eTransform transform)
    {
        m_transform = transform;
        return *this;
    }

    RenderSourceBuilder &setOffset(eTransform offset)
    {
        m_offset = offset;
        return *this;
    }

    RenderSourceBuilder &setModel(Model *model)
    {
        m_model = model;
        return *this;
    }

    RenderSourceBuilder &setAnimator(Animator *animator)
    {
        m_animator = animator;
        return *this;
    }

    RenderSourceBuilder &setTransformLink(TransformLink *transformLink)
    {
        m_transformLink = transformLink;
        return *this;
    }

    RenderSourceBuilder &setMergedPBRTextures(bool mergedPBRTextures)
    {
        m_mergedPBRTextures = mergedPBRTextures;
        return *this;
    }

    RenderSource *build()
    {
        return new RenderSource(m_type, m_transform, m_offset, m_model, m_animator, m_transformLink, m_mergedPBRTextures);
    }

private:
    ShaderType m_type;
    eTransform m_transform;
    eTransform m_offset;
    Model *m_model = nullptr;
    Animator *m_animator = nullptr;
    TransformLink *m_transformLink = nullptr;
    bool m_mergedPBRTextures;
};

class RenderManager
{
public:
    RenderManager(ShaderManager *shaderManager, Camera *camera, Model *cube, Model *quad, unsigned int quad_vao);
    ~RenderManager();

    ShaderManager *m_shaderManager;
    Camera *m_camera;
    // TODO: move?
    Model *cube;
    Model *quad;
    unsigned int quad_vao;

    PbrManager *m_pbrManager;
    ShadowManager *m_shadowManager;
    ShadowmapManager *m_shadowmapManager;
    PostProcess *m_postProcess;

    // TODO: animation support with preprocessor - pbr, basic, depth
    Shader pbrShader;
    Shader animShader; // TODO: basicShader
    Shader depthShader;
    Shader animDepthShader; // TODO: remove

    Shader terrainPBRShader;
    Shader terrainBasicShader;
    Shader terrainDepthShader;

    // Skybox
    Shader cubemapShader;
    Shader postProcessShader;

    // PBR Shaders
    Shader hdrToCubemapShader;
    Shader irradianceShader;
    Shader prefilterShader;
    Shader brdfShader;

    int m_screenW, m_screenH;
    glm::mat4 m_projection;
    glm::mat4 m_view;
    glm::mat4 m_viewProjection;
    glm::mat4 m_inverseDepthViewMatrix;
    glm::vec4 m_frustumDistances;

    // TODO: light source
    glm::vec3 m_sunColor = glm::vec3(3.5f, 4.1f, 4.5f);
    float m_sunIntensity = 1.5f;
    float m_lightPower = 10.0;

    glm::vec3 m_shadowBias = glm::vec3(0.020, 0.023, 0.005);
    bool m_cullFront = false;

    void updateTransforms();
    void setupFrame(GLFWwindow *window);
    void renderDepth();
    void renderOpaque();
    void renderBlend();
    void renderTransmission();
    void renderPostProcess();

    void addSource(RenderSource *source);
    RenderTerrainSource *addTerrainSource(ShaderType type, eTransform transform, Terrain *terrain);
    void addParticleSource(RenderParticleSource *source);

    std::vector<RenderSource *> m_pbrSources;
    std::vector<RenderSource *> m_basicSources;
    std::vector<RenderSource *> m_linkSources;

    std::vector<RenderTerrainSource *> m_pbrTerrainSources;
    std::vector<RenderTerrainSource *> m_basicTerrainSources;

    std::vector<RenderParticleSource *> m_particleSources;
};

#endif /* render_manager_hpp */
