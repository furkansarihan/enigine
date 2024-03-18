#include "physics_ui.h"

void PhysicsWorldUI::renderDepth()
{
}

void PhysicsWorldUI::renderColor()
{
    if (!m_selectedObject)
        return;

    eTransform transform;
    transform.setPosition(BulletGLM::getGLMVec3(m_selectedObject->getWorldTransform().getOrigin()));
    transform.setScale(glm::vec3(0.05));

    Shader &shader = m_renderManager->simpleDeferredShader;

    shader.use();
    shader.setMat4("u_meshOffset", glm::mat4(1.f));
    shader.setMat4("view", m_renderManager->m_view);
    shader.setMat4("projection", m_renderManager->m_projection);
    shader.setVec3("lightDirection", m_renderManager->m_shadowManager->m_lightPos);
    shader.setVec3("u_albedo", glm::vec4(1.f, 0.f, 1.f, 1.f));
    shader.setMat4("model", transform.getModelMatrix());

    // glDisable(GL_DEPTH_TEST);
    m_renderManager->sphere->draw(shader);
    // glEnable(GL_DEPTH_TEST);
}

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

    ImGui::Separator();

    renderRigidbodies();
    renderSelectedObject();
}

void PhysicsWorldUI::renderSelectedObject()
{
    if (!m_selectedObject)
        return;

    ImGui::Text("Selected Object");

    if (m_selectedObject->getInternalType() == btCollisionObject::CO_RIGID_BODY)
    {
        btRigidBody *rigidBody = btRigidBody::upcast(m_selectedObject);
        if (rigidBody)
            renderRigidbody(rigidBody, 0);
    }
    else if (m_selectedObject->getInternalType() == btCollisionObject::CO_SOFT_BODY)
    {
        btSoftBody *softBody = btSoftBody::upcast(m_selectedObject);
        if (softBody)
            renderSoftbody(softBody, 0);
    }

    ImGui::Separator();
}

void PhysicsWorldUI::renderRigidbodies()
{
    ImGui::Text("Collision Objects");

    int objectCount = m_physicsWorld->m_dynamicsWorld->getNumCollisionObjects();
    if (objectCount == 0)
        return;

    float maxHeight = 200.0f;
    float height = ImGui::GetFrameHeight() * objectCount;
    if (height > maxHeight)
        height = maxHeight;

    ImGui::BeginChild("PhysicsWorldUI::renderRigidbodies", ImVec2(0, height), false);

    for (int i = 0; i < objectCount; i++)
    {
        btCollisionObject *obj = m_physicsWorld->m_dynamicsWorld->getCollisionObjectArray()[i];
        if (!obj)
            continue;

        std::stringstream ss;
        ss << obj;

        if (ImGui::Selectable(ss.str().c_str(), m_selectedObject == obj))
        {
            if (m_selectedObject == obj)
                m_selectedObject = nullptr;
            else
                m_selectedObject = obj;
        }
    }

    ImGui::EndChild();

    ImGui::Separator();
}

void PhysicsWorldUI::renderRigidbody(btRigidBody *body, int i)
{
    btCollisionShape *shape = body->getCollisionShape();

    glm::vec3 pos = BulletGLM::getGLMVec3(body->getWorldTransform().getOrigin());
    if (VectorUI::renderVec3(("position##renderRigidbodies" + std::to_string(i)).c_str(), pos, 0.1f))
    {
        btTransform tr;
        tr.setIdentity();
        tr.setOrigin(BulletGLM::getBulletVec3(pos));
        body->setWorldTransform(tr);
    }

    float mass = body->getMass();
    if (ImGui::DragFloat(("mass##renderRigidbodies" + std::to_string(i)).c_str(), &mass, 0.1))
    {
        btVector3 interia;
        body->getCollisionShape()->calculateLocalInertia(mass, interia);
        body->setMassProps(mass, interia);
    }
    float friction = body->getFriction();
    if (ImGui::DragFloat(("friction##renderRigidbodies" + std::to_string(i)).c_str(), &friction, 0.1))
    {
        body->setFriction(friction);
    }
    float restitution = body->getRestitution();
    if (ImGui::DragFloat(("restitution##renderRigidbodies" + std::to_string(i)).c_str(), &restitution, 0.1))
    {
        body->setRestitution(restitution);
    }
    float linearDamping = body->getLinearDamping();
    float angularDamping = body->getAngularDamping();
    if (ImGui::DragFloat(("linearDamping##renderRigidbodies" + std::to_string(i)).c_str(), &linearDamping, 0.1))
    {
        body->setDamping(linearDamping, angularDamping);
    }
    if (ImGui::DragFloat(("angularDamping##renderRigidbodies" + std::to_string(i)).c_str(), &angularDamping, 0.1))
    {
        body->setDamping(linearDamping, angularDamping);
    }

    btVector3 size = body->getCollisionShape()->getLocalScaling();
    if (VectorUI::renderVec3((std::string("localScalingSize##") + std::to_string(i)).c_str(), size, 0.1f))
        body->getCollisionShape()->setLocalScaling(size);

    bool active = body->isActive();
    if (ImGui::Checkbox("Activation", &active))
    {
        if (active)
            body->activate(true);
        else
            body->setActivationState(0);
    }
}

void PhysicsWorldUI::renderSoftbody(btSoftBody *body, int i)
{
    float contactStiffness = body->getContactStiffness();
    float contactDamping = body->getContactDamping();
    if (ImGui::DragFloat(("contactStiffness##renderSoftbody" + std::to_string(i)).c_str(), &contactStiffness, 0.1f))
        body->setContactStiffnessAndDamping(contactStiffness, contactDamping);
    if (ImGui::DragFloat(("contactDamping##renderSoftbody" + std::to_string(i)).c_str(), &contactDamping, 0.1f))
        body->setContactStiffnessAndDamping(contactStiffness, contactDamping);

    ImGui::Combo(("Aerodynamic Model##renderSoftbody" + std::to_string(i)).c_str(), (int *)&body->m_cfg.aeromodel, "V_Point\0V_TwoSided\0V_OneSided\0F_TwoSided\0F_OneSided\0\0");

    ImGui::DragFloat(("Area/Angular stiffness coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_materials[0]->m_kAST, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Linear stiffness coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_materials[0]->m_kLST, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Volume stiffness coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_materials[0]->m_kVST, 0.01f, 0.f, 1.f);

    if (ImGui::DragFloat(("Velocities Correction Factor##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kVCF, 0.01f, 0.f, FLT_MAX))
    {
        body->generateClusters(0);
    }
    ImGui::DragFloat(("Damping Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kDP, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Drag Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kDG, 0.01f, 0.f, FLT_MAX);
    ImGui::DragFloat(("Lift Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kLF, 0.01f, 0.f, FLT_MAX);
    ImGui::DragFloat(("Pressure Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kPR, 0.01f, -FLT_MAX, FLT_MAX);
    ImGui::DragFloat(("Volume Conservation Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kVC, 0.01f, 0.f, FLT_MAX);
    ImGui::DragFloat(("Dynamic Friction Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kDF, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Pose Matching Coefficient##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kMT, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Rigid Contacts Hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kCHR, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Kinetic Contacts Hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kKHR, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Soft Contacts Hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSHR, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Anchors Hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kAHR, 0.01f, 0.f, 1.f);

    ImGui::DragFloat(("Soft vs kinetic hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSKHR_CL, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Soft vs soft hardness##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSSHR_CL, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Soft vs rigid impulse split kSR_SPLT_CL##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSR_SPLT_CL, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Soft vs rigid impulse split kSK_SPLT_CL##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSK_SPLT_CL, 0.01f, 0.f, 1.f);
    ImGui::DragFloat(("Soft vs rigid impulse split kSS_SPLT_CL##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.kSS_SPLT_CL, 0.01f, 0.f, 1.f);

    ImGui::DragFloat(("Maximum volume ratio for pose##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.maxvolume, 0.01f, 0.f, FLT_MAX);
    ImGui::DragFloat(("Time scale##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.timescale, 0.01f, 0.f, FLT_MAX);

    ImGui::DragInt(("Velocities solver iterations##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.viterations, 1);
    ImGui::DragInt(("Positions solver iterations##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.piterations, 1);
    ImGui::DragInt(("Drift solver iterations##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.diterations, 1);
    ImGui::DragInt(("Cluster solver iterations##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.citerations, 1);

    ImGui::DragFloat(("Deformable air drag##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.drag, 0.01f, 0.f, FLT_MAX);
    ImGui::DragFloat(("Maximum principle first Piola stress##renderSoftbody" + std::to_string(i)).c_str(), &body->m_cfg.m_maxStress, 0.01f, 0.f, FLT_MAX);
}
