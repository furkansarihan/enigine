#include "character_ui.h"

void CharacterUI::render()
{
    if (!ImGui::CollapsingHeader("Character", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    // TODO: change camera min pitch
    if (ImGui::Checkbox("m_firstPerson", &m_character->m_firstPerson))
    {
        if (m_character->m_firstPerson)
        {
            m_character->m_followDistance = -1.0f;
            m_character->m_followOffset.y = 3.3f;
        }
        else
        {
            m_character->m_followDistance = 10.0f;
            m_character->m_followOffset.y = 1.5f;
        }
    }
    ImGui::Checkbox("m_turnLocked", &m_controller->m_turnLocked);
    ImGui::Checkbox("m_controlCharacter", &m_character->m_controlCharacter);
    ImGui::Checkbox("m_followCharacter", &m_character->m_followCharacter);
    ImGui::DragFloat("m_followDistance", &m_character->m_followDistance, 0.1f);
    ImGui::DragFloat("m_followOffsetY", &m_character->m_followOffset.y, 0.1f);
    ImGui::DragFloat("m_followHeightFactor", &m_character->m_followHeightFactor, 0.1f);
    ImGui::Separator();
    if (ImGui::CollapsingHeader("m_walkSpeed", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        renderSpeedLimiter(m_character->m_controller->m_walkSpeed, "m_walkSpeed");
    if (ImGui::CollapsingHeader("m_runSpeed", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        renderSpeedLimiter(m_character->m_controller->m_runSpeed, "m_runSpeed");
    ImGui::Text("m_lookDir: (%.1f, %.1f)", m_controller->m_lookDir.x, m_controller->m_lookDir.z);
    ImGui::Text("m_moving: %d", m_controller->m_moving);
    ImGui::Text("m_jumping: %d", m_controller->m_jumping);
    ImGui::Text("m_falling: %d", m_controller->m_falling);
    ImGui::Text("m_onGround: %d", m_controller->m_onGround);
    ImGui::Text("m_running: %d", m_controller->m_running);
    ImGui::Text("m_turning: %d", m_controller->m_turning);
    ImGui::Text("m_turnFactor: %.3f", m_controller->m_turnFactor);
    ImGui::Text("m_signedMoveAngle: %.3f", m_controller->m_signedMoveAngle);
    ImGui::Text("m_dotFront: %.3f", m_controller->m_dotFront);
    ImGui::DragFloat("m_moveAngleForce", &m_controller->m_moveAngleForce, 0.1f, 0);
    ImGui::DragFloat("m_blendSpeed", &m_character->m_blendSpeed, 0.1f, 0);
    ImGui::DragFloat("m_turnAnimMult", &m_controller->m_turnAnimMult, 0.1f, 0);
    // ImGui::Text("m_det: %.3f", m_controller->m_det);
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
    ImGui::DragFloat("m_toIdleForce", &m_controller->m_toIdleForce, 0.1f, 0);
    ImGui::DragFloat("m_toIdleForceHoriz", &m_controller->m_toIdleForceHoriz, 0.1f, 0);
    ImGui::DragFloat("m_groundTreshold", &m_controller->m_groundTreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnTreshold", &m_controller->m_turnTreshold, 0.001f, 0);
    ImGui::DragFloat("m_walkToRunAnimTreshold", &m_controller->m_walkToRunAnimTreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnAnimForce", &m_controller->m_turnAnimForce, 0.01f, 0);
    ImGui::DragFloat("m_turnAnimMaxFactor", &m_controller->m_turnAnimMaxFactor, 0.1f, 0);
    ImGui::DragFloat("m_floatElevation", &m_controller->m_floatElevation, 0.001f, 0);
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

void CharacterUI::renderSpeedLimiter(SpeedLimiter &speedLimiter, std::string name)
{
    const int numPoints = 100;
    float values[numPoints];
    const float rangeMin = -M_PI;
    const float rangeMax = M_PI;

    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float>(i) / (numPoints - 1);
        float value = rangeMin + t * (rangeMax - rangeMin);
        values[i] = speedLimiter.getSpeed(value);
    }
    ImGui::PlotLines("Graph", values, numPoints);
    for (int i = 0; i < speedLimiter.m_points.size(); i++)
    {
        LimiterPoint &point = speedLimiter.m_points[i];
        std::string nameAngle = "::angle";
        std::string nameHeight = "::height";
        std::string nameLength = "::length";

        ImGui::DragFloat((name + nameAngle + std::to_string(i)).c_str(), &point.angle, 0.01f, 0.f, 2.f * M_PI);
        ImGui::DragFloat((name + nameHeight + std::to_string(i)).c_str(), &point.heightFactor, 0.1f);
        ImGui::DragFloat((name + nameLength + std::to_string(i)).c_str(), &point.lengthFactor, 0.1f);
        ImGui::Separator();
    }
}
