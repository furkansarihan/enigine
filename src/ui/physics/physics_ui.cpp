#include "physics_ui.h"

void PhysicsWorldUI::render()
{
    if (!ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::DragInt("m_maxSubSteps", &m_physicsWorld->m_maxSubSteps);
    bool debugEnabled = m_debugDrawer->getDebugMode();
    if (ImGui::Checkbox("debugEnabled", &debugEnabled))
    {
        m_debugDrawer->setDebugMode(debugEnabled ? btIDebugDraw::DBG_DrawWireframe |
                                                       btIDebugDraw::DBG_DrawConstraints |
                                                       btIDebugDraw::DBG_DrawConstraintLimits
                                                 : btIDebugDraw::DBG_NoDebug);
    }
    int lines = m_debugDrawer->getLines().size();
    ImGui::DragInt("lines", &lines);
}
