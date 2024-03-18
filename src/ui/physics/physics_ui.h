#ifndef physics_world_ui_hpp
#define physics_world_ui_hpp

#include "../base_ui.h"
#include "../../post_process/post_process.h"
#include "../../physics_world/physics_world.h"
#include "../../physics_world/debug_drawer/debug_drawer.h"
#include "../../shader_manager/shader_manager.h"
#include "../../transform/transform.h"
#include "../../render_manager/render_manager.h"

class PhysicsWorldUI : public BaseUI, public Renderable
{
private:
    RenderManager *m_renderManager;
    PhysicsWorld *m_physicsWorld;
    DebugDrawer *m_debugDrawer;

    btCollisionObject *m_selectedObject;

public:
    PhysicsWorldUI(RenderManager *renderManager, PhysicsWorld *physicsWorld, DebugDrawer *debugDrawer)
        : m_renderManager(renderManager),
          m_physicsWorld(physicsWorld),
          m_debugDrawer(debugDrawer)
    {
    }

    void renderDepth() override;
    void renderColor() override;

    void render() override;
    void renderSelectedObject();
    void renderRigidbodies();
    void renderRigidbody(btRigidBody *body, int i);
    void renderSoftbody(btSoftBody *body, int i);
};

#endif /* physics_world_ui_hpp */
