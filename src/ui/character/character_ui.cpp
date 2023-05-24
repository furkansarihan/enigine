#include "character_ui.h"

void CharacterUI::render()
{
    if (!ImGui::CollapsingHeader("Character", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    ImGui::Text("m_moveDir: (%.1f, %.1f)", m_controller->m_moveDir.x, m_controller->m_moveDir.z);
    ImGui::Text("m_moving: %d", m_controller->m_moving);
    ImGui::Text("m_jumping: %d", m_controller->m_jumping);
    ImGui::Text("m_falling: %d", m_controller->m_falling);
    ImGui::Text("m_onGround: %d", m_controller->m_onGround);
    ImGui::Text("m_running: %d", m_controller->m_running);
    ImGui::Text("m_turning: %d", m_controller->m_turning);
    ImGui::Text("m_turnFactor: %.2f", m_controller->m_turnFactor);
    btVector3 linearVelocity = m_rb->getLinearVelocity();
    btVector3 angularVelocity = m_rb->getAngularVelocity();
    ImGui::Text("linearVelocity: (%.1f, %.1f, %.1f)", linearVelocity.getX(), linearVelocity.getY(), linearVelocity.getZ());
    ImGui::Text("angularVelocity: (%.1f, %.1f, %.1f)", angularVelocity.getX(), angularVelocity.getY(), angularVelocity.getZ());
    ImGui::Text("linearSpeed: %.1f", linearVelocity.distance(btVector3(0, 0, 0)));
    ImGui::Text("m_speed: %.3f", m_controller->m_speed);
    ImGui::Text("m_elevationDistance: %.3f", m_controller->m_elevationDistance);
    ImGui::Text("m_speedAtJumpStart: %.3f", m_controller->m_speedAtJumpStart);
    ImGui::DragFloat("m_moveForce", &m_controller->m_moveForce, 0.1f, 0);
    ImGui::DragFloat("m_jumpForce", &m_controller->m_jumpForce, 0.1f, 0);
    ImGui::DragFloat("m_turnForce", &m_controller->m_turnForce, 0.001f, 0);
    ImGui::DragFloat("m_maxWalkSpeed", &m_controller->m_maxWalkSpeed, 0.1f, 0);
    ImGui::DragFloat("m_maxRunSpeed", &m_controller->m_maxRunSpeed, 0.1f, 0);
    ImGui::DragFloat("m_toIdleForce", &m_controller->m_toIdleForce, 0.1f, 0);
    ImGui::DragFloat("m_toIdleForceHoriz", &m_controller->m_toIdleForceHoriz, 0.1f, 0);
    ImGui::DragFloat("m_groundTreshold", &m_controller->m_groundTreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnTreshold", &m_controller->m_turnTreshold, 0.001f, 0);
    ImGui::DragFloat("m_walkToRunAnimTreshold", &m_controller->m_walkToRunAnimTreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnAnimForce", &m_controller->m_turnAnimForce, 0.01f, 0);
    ImGui::DragFloat("m_turnAnimMaxFactor", &m_controller->m_turnAnimMaxFactor, 0.1f, 0);
    float gravityY = m_rb->getGravity().getY();
    if (ImGui::DragFloat("gravity", &gravityY, 0.1f))
    {
        m_rb->setGravity(btVector3(0, gravityY, 0));
    }
    float sX = m_rb->getWorldTransform().getOrigin().getX();
    float sY = m_rb->getWorldTransform().getOrigin().getY();
    float sZ = m_rb->getWorldTransform().getOrigin().getZ();
    if (ImGui::DragFloat("sX", &sX, 0.1f))
    {
        m_rb->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
        m_rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("sY", &sY, 0.1))
    {
        m_rb->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
        m_rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    if (ImGui::DragFloat("sZ", &sZ, 0.1))
    {
        m_rb->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
        m_rb->setLinearVelocity(btVector3(0, 0, 0));
    }
    float characterMass = m_rb->getMass();
    if (ImGui::DragFloat("characterMass", &characterMass, 0.1))
    {
        btVector3 interia;
        m_rb->getCollisionShape()->calculateLocalInertia(characterMass, interia);
        m_rb->setMassProps(characterMass, interia);
    }
    float friction = m_rb->getFriction();
    if (ImGui::DragFloat("friction", &friction, 0.1))
    {
        m_rb->setFriction(friction);
    }
    float linearDamping = m_rb->getLinearDamping();
    float angularDamping = m_rb->getLinearDamping();
    if (ImGui::DragFloat("linearDamping", &linearDamping, 0.1))
    {
        m_rb->setDamping(linearDamping, angularDamping);
    }
    if (ImGui::DragFloat("angularDamping", &angularDamping, 0.1))
    {
        m_rb->setDamping(linearDamping, angularDamping);
    }
}
