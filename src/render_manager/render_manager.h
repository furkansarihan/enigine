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
#include "../post_process/bloom_manager.h"
#include "../transform_link/transform_link.h"
#include "../particle_engine/particle_engine.h"
#include "../culling_manager/culling_manager.h"
#include "../utils/common.h"

#include "g_buffer.h"

enum ShaderType
{
    pbr,
    basic
};

class RenderManager;
class RenderSource
{
public:
    RenderManager *m_renderManager = nullptr;
    eTransform transform;
    eTransform offset;
    Model *model = nullptr;
    Animator *animator = nullptr;
    TransformLink *transformLink = nullptr;
    // TODO: auto detect
    bool aoRoughMetalMap = false;
    int cullIndex = -1;

    RenderSource(eTransform transform, eTransform offset, Model *model, Animator *animator, TransformLink *transformLink, bool aoRoughMetalMap)
        : transform(transform),
          offset(offset),
          model(model),
          animator(animator),
          transformLink(transformLink),
          aoRoughMetalMap(aoRoughMetalMap){};

    void setTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
    void setModelMatrix(glm::mat4 modelMatrix);
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
    RenderSourceBuilder() {}

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

    RenderSourceBuilder &setAoRoughMetalMap(bool aoRoughMetalMap)
    {
        m_aoRoughMetalMap = aoRoughMetalMap;
        return *this;
    }

    RenderSource *build()
    {
        return new RenderSource(m_transform, m_offset, m_model, m_animator, m_transformLink, m_aoRoughMetalMap);
    }

private:
    eTransform m_transform;
    eTransform m_offset;
    Model *m_model = nullptr;
    Animator *m_animator = nullptr;
    TransformLink *m_transformLink = nullptr;
    bool m_aoRoughMetalMap;
};

// TODO: as stride
struct LightSource
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity = 10.f;

    float radius;
    float linear;
    float quadratic;

    LightSource(glm::vec3 position, glm::vec3 color, float intensity)
        : position(position),
          color(color),
          intensity(intensity),
          radius(1.f),
          linear(1.f),
          quadratic(1.f){};
};

struct LightInstance
{
    glm::mat4 model;
    glm::vec3 lightColor;
    float radius;
    float linear;
    float quadratic;

    LightInstance(){};
};

class RenderManager
{
public:
    RenderManager(ShaderManager *shaderManager, Camera *camera, Model *cube, Model *quad, Model *sphere, unsigned int quad_vao);
    ~RenderManager();

    ShaderManager *m_shaderManager;
    Camera *m_camera;
    Camera *m_debugCamera;
    // TODO: move?
    Model *cube;
    Model *quad;
    Model *sphere;
    unsigned int quad_vao;

    PbrManager *m_pbrManager;
    ShadowManager *m_shadowManager;
    ShadowmapManager *m_shadowmapManager;
    CullingManager *m_cullingManager;
    GBuffer *m_gBuffer;
    PostProcess *m_postProcess;
    BloomManager *m_bloomManager;
    bool m_debugCulling = false;
    bool m_drawCullingAabb = false;

    std::vector<RenderSource *> m_visiblePbrSources;
    std::vector<RenderSource *> m_visiblePbrAnimSources;

    std::vector<RenderSource *> m_pbrSources;
    std::vector<RenderSource *> m_linkSources;
    std::vector<RenderTerrainSource *> m_pbrTerrainSources;
    std::vector<RenderTerrainSource *> m_basicTerrainSources;
    std::vector<RenderParticleSource *> m_particleSources;

    std::vector<LightSource> m_pointLights;

    Shader pbrDeferredPre;
    Shader pbrDeferredPreAnim;
    Shader pbrDeferredAfter;
    Shader pbrDeferredPointLight;
    Shader pbrTransmission;
    Shader depthShader;
    Shader depthShaderAnim;
    Shader lightVolume;
    Shader lightVolumeDebug;

    Shader terrainPBRShader;
    Shader terrainBasicShader;
    Shader terrainDepthShader;

    // Skybox
    Shader skyboxShader;
    Shader postProcessShader;

    // PBR Shaders
    Shader hdrToCubemapShader;
    Shader irradianceShader;
    Shader prefilterShader;
    Shader brdfShader;

    Shader downsampleShader;
    Shader upsampleShader;

    int m_screenW, m_screenH;
    glm::mat4 m_projection;
    glm::mat4 m_view;
    glm::mat4 m_viewProjection;
    glm::mat4 m_depthViewMatrix;
    glm::mat4 m_inverseDepthViewMatrix;
    glm::vec4 m_frustumDistances;
    glm::mat4 m_cullProjection;
    glm::mat4 m_cullView;
    glm::mat4 m_cullViewProjection;
    glm::vec3 m_cullViewPos;

    // TODO: light source
    glm::vec3 m_sunColor = glm::vec3(3.5f, 4.1f, 4.5f);
    float m_sunIntensity = 1.5f;
    float m_lightPower = 10.0;

    glm::vec3 m_shadowBias = glm::vec3(0.020, 0.023, 0.005);
    bool m_cullFront = false;

    bool m_lightAreaDebug = false;
    bool m_lightSurfaceDebug = false;

    // instanced lights
    unsigned int m_lightArrayBuffer;
    std::vector<LightInstance> m_lightBufferList;

    void updateTransforms();
    void setupFrame(GLFWwindow *window);
    void renderDepth();
    void renderOpaque();
    void renderDeferredShading();
    void renderBlend();
    void renderTransmission();
    void renderPostProcess();

    void addSource(RenderSource *source);
    RenderTerrainSource *addTerrainSource(ShaderType type, eTransform transform, Terrain *terrain);
    void addParticleSource(RenderParticleSource *source);
    void addLight(LightSource light);

private:
    void setupLights();
    void renderLightVolumes(std::vector<LightSource> &lights, bool camInsideVolume);
    void updateLightBuffer(std::vector<LightSource> &lights);
    bool inShadowFrustum(RenderSource *source, int frustumIndex);
};

#endif /* render_manager_hpp */
