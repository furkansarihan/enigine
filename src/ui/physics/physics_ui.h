#ifndef physics_world_ui_hpp
#define physics_world_ui_hpp

#include "../base_ui.h"
#include "../../post_process/post_process.h"
#include "../../physics_world/physics_world.h"
#include "../../physics_world/debug_drawer/debug_drawer.h"
#include "../../shader_manager/shader_manager.h"
#include "../../transform/transform.h"

class PhysicsWorldUI : public BaseUI
{
private:
    PhysicsWorld *m_physicsWorld;
    DebugDrawer *m_debugDrawer;

public:
    PhysicsWorldUI(PhysicsWorld *physicsWorld, DebugDrawer *debugDrawer)
        : m_physicsWorld(physicsWorld),
          m_debugDrawer(debugDrawer)
    {
    }

    void render() override;
    void renderRigidbodies();
    void renderRigidbody(btRigidBody *body, int i);
    void renderSoftbody(btSoftBody *body, int i);
};

#endif /* physics_world_ui_hpp */
