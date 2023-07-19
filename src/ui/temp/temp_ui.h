#ifndef temp_ui_hpp
#define temp_ui_hpp

#include "../base_ui.h"
#include "../../post_process/post_process.h"
#include "../../physics_world/physics_world.h"
#include "../../physics_world/debug_drawer/debug_drawer.h"
#include "../../shader_manager/shader_manager.h"
#include "../../transform/transform.h"

class TempUI : public BaseUI
{
private:
    PostProcess *m_postProcess;
    PhysicsWorld *m_physicsWorld;
    DebugDrawer *m_debugDrawer;
    ShaderManager *m_shaderManager;

public:
    TempUI(PostProcess *postProcess, PhysicsWorld *physicsWorld, DebugDrawer *debugDrawer, ShaderManager *shaderManager)
        : m_postProcess(postProcess),
          m_physicsWorld(physicsWorld),
          m_debugDrawer(debugDrawer),
          m_shaderManager(shaderManager)
    {
        m_shelterTransform.m_position = glm::vec3(103.f, 1.8f, 260.f);
        m_shelterTransform.m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        m_shelterTransform.m_scale = glm::vec3(1.f, 1.f, 1.f);

        m_towerTransform.m_position = glm::vec3(112.f, 18.2f, 233.f);
        m_towerTransform.m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        m_towerTransform.m_scale = glm::vec3(.1f, .1f, .1f);
    }

    glm::vec3 m_sunColor = glm::vec3(3.5f, 4.1f, 4.5f);
    float m_sunIntensity = 1.5f;
    float m_lightColor[3] = {0.1f, 0.1f, 0.1f};
    float m_ambientColor[3] = {0.4f, 0.4f, 0.4f};
    float m_specularColor[3] = {0.15f, 0.15f, 0.15f};
    float m_lightPower = 10.0;
    float m_radius = 12.0;
    float m_speed = 2.0;

    bool m_cullFront = false;

    float m_deltaTime = 0.f;

    Transform m_shelterTransform;
    Transform m_towerTransform;

    void render() override;
};

#endif /* temp_ui_hpp */
