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
#include "../resource_manager/resource_manager.h"
#include "../utils/common.h"

#include "g_buffer.h"
#include "ssao.h"

enum class FaceCullType
{
    backFaces,
    frontFaces,
    none,
    COUNT
};

enum class PolygonMode
{
    point,
    line,
    fill,
    COUNT
};

class RenderManager;
class RenderSource
{
public:
    RenderManager *m_renderManager = nullptr;
    eTransform transform;
    eTransform offset;
    glm::mat4 modelMatrix;
    FaceCullType faceCullType;
    PolygonMode polygonMode;
    Model *model = nullptr;
    Animator *animator = nullptr;
    TransformLink *transformLink = nullptr;
    int cullIndex = -1;

    RenderSource(eTransform transform, eTransform offset, FaceCullType faceCullType, Model *model, Animator *animator, TransformLink *transformLink)
        : transform(transform),
          offset(offset),
          modelMatrix(glm::mat4(1.f)),
          faceCullType(faceCullType),
          polygonMode(PolygonMode::fill),
          model(model),
          animator(animator),
          transformLink(transformLink){};

    void updateModelMatrix();
};

// TODO: remove?
class RenderParticleSource
{
public:
    Shader *shader;
    Model *model;
    ParticleEngine *particleEngine;
    TransformLink *transformLink = nullptr;

    RenderParticleSource(Shader *shader, Model *model, ParticleEngine *particleEngine)
        : shader(shader),
          model(model),
          particleEngine(particleEngine){};

    RenderParticleSource(Shader *shader, Model *model, ParticleEngine *particleEngine, TransformLink *transformLink)
        : shader(shader),
          model(model),
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

    RenderSourceBuilder &setFaceCullType(FaceCullType faceCullType)
    {
        facecullType = faceCullType;
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

    RenderSource *build()
    {
        return new RenderSource(m_transform, m_offset, facecullType, m_model, m_animator, m_transformLink);
    }

private:
    eTransform m_transform;
    eTransform m_offset;
    FaceCullType facecullType = FaceCullType::backFaces;
    Model *m_model = nullptr;
    Animator *m_animator = nullptr;
    TransformLink *m_transformLink = nullptr;
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
          linear(0.09f),
          quadratic(0.032f){};
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

class Renderable
{
public:
    virtual ~Renderable() {}

    virtual void renderDepth() = 0;
    virtual void renderColor() = 0;
};

class ForwardRenderable
{
public:
    virtual ~ForwardRenderable() {}

    virtual void renderDepth() = 0;
    virtual void renderColor() = 0;
};

class TransparentRenderable
{
public:
    virtual ~TransparentRenderable() {}

    virtual void renderTransparent() = 0;
};

class RenderManager
{
public:
    RenderManager(ShaderManager *shaderManager, ResourceManager *resourceManager, Camera *camera);
    ~RenderManager();

    ShaderManager *m_shaderManager;
    ResourceManager *m_resourceManager;
    Camera *m_camera;
    Camera *m_debugCamera;

    // objects
    Model *pointLightVolume;
    unsigned int vbo, vao, ebo;
    unsigned int quad_vao, quad_vbo, quad_ebo;

    // common objects
    Model *cube;
    Model *quad;
    Model *sphere;
    Model *icosahedron;

    PbrManager *m_pbrManager;
    ShadowManager *m_shadowManager;
    ShadowmapManager *m_shadowmapManager;
    CullingManager *m_cullingManager;
    GBuffer *m_gBuffer;
    SSAO *m_ssao;
    PostProcess *m_postProcess;
    BloomManager *m_bloomManager;
    glm::mat4 m_originTransform;
    bool m_debugCulling = false;
    bool m_drawCullingAabb = false;

    std::vector<RenderSource *> m_visiblePbrSources;
    std::vector<RenderSource *> m_visiblePbrAnimSources;
    std::vector<Renderable *> m_renderables;
    std::vector<ForwardRenderable *> m_forwardRenderables;
    std::vector<TransparentRenderable *> m_transparentRenderables;

    std::vector<RenderSource *> m_pbrSources;
    std::vector<RenderSource *> m_linkSources;
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
    Shader shaderSSAO, shaderSSAOBlur;

    // common shaders
    Shader simpleShader, simpleDeferredShader, lineShader, textureArrayShader;

    // skybox
    Shader skyboxShader;
    Shader postProcessShader;

    // PBR shaders
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

    // updated for each cascade
    int m_frustumIndex;
    glm::mat4 m_depthP;
    glm::mat4 m_depthVP;
    glm::vec3 m_depthNearPlaneCenter;

    // TODO: light source
    glm::vec3 m_sunColor = glm::vec3(1.f, 1.f, 1.f);
    float m_sunIntensity = 10.f;
    float m_lightPower = 10.0;

    glm::vec3 m_shadowBias;
    bool m_cullFront = false;

    bool m_lightAreaDebug = false;
    bool m_lightSurfaceDebug = false;

    // instanced lights
    unsigned int m_lightArrayBuffer;
    std::vector<LightInstance> m_lightBufferList;

    float fogMaxDist = 10000.0f;
    float fogMinDist = 4500.0f;
    glm::vec4 fogColor = glm::vec4(1.f, 1.f, 1.f, 0.75f);

    void updateTransforms();
    void setupFrame(GLFWwindow *window);
    void renderDepth();
    void renderOpaque();
    void renderSSAO();
    void renderDeferredShading();
    void renderBlend();
    void renderTransmission();
    void renderPostProcess();

    void updateEnvironmentTexture(Texture *newTexture);

    void addSource(RenderSource *source);
    void removeSource(RenderSource *source);
    void addParticleSource(RenderParticleSource *source);
    void addLight(LightSource light);

    void addRenderable(Renderable *renderable);
    void removeRenderable(Renderable *renderable);

    glm::vec3 getWorldOrigin();
    void setWorldOrigin(glm::vec3 newWorldOrigin);

private:
    // TODO: chain with camera?
    glm::vec3 m_worldOrigin;

    void setupLights();
    void renderLightVolumes(std::vector<LightSource> &lights, bool camInsideVolume);
    void updateLightBuffer(std::vector<LightSource> &lights);
    bool inShadowFrustum(RenderSource *source, int frustumIndex);
};

#endif /* render_manager_hpp */
