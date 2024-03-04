#include "character_ui.h"

void CharacterUI::render()
{
    if (!ImGui::CollapsingHeader("Character", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    renderState();
    renderMovement();
    renderPhysics();
    renderController();
    renderAttachments();

    ImGui::DragFloat("m_scale", &m_character->m_scale, 0.1f);
    VectorUI::renderVec3("m_rotation", m_character->m_rotation, 0.1f);
    ImGui::Separator();
    VectorUI::renderVec3("m_followOffsetNormal", m_character->m_followOffsetNormal, 0.1f);
    VectorUI::renderVec3("m_followOffsetAim", m_character->m_followOffsetAim, 0.1f);
    ImGui::DragFloat("m_followOffsetFactor", &m_character->m_followOffsetFactor, 0.1f);
    ImGui::Separator();

    if (ImGui::TreeNode("Footstep"))
    {
        ImGui::DragFloat("m_walkStepFreq", &m_character->m_walkStepFreq, 0.1f);
        ImGui::DragFloat("m_runStepFreq", &m_character->m_runStepFreq, 0.1f);
        ImGui::DragFloat("m_stopBlendSpeed", &m_character->m_stopBlendSpeed, 0.001f);
        ImGui::Text("m_walkStepFreq: %.5f", m_character->m_walkStepFreq);
        ImGui::Text("m_runStepFreq: %.5f", m_character->m_runStepFreq);
        ImGui::Text("m_walkPerc: %.3f", m_character->m_walkPerc);
        ImGui::Text("m_runPerc: %.3f", m_character->m_runPerc);
        ImGui::Text("m_walkAnimSpeed: %.3f", m_character->m_walkAnimSpeed);
        ImGui::Text("m_runAnimSpeed: %.3f", m_character->m_runAnimSpeed);
        ImGui::TreePop();
    }
}

void CharacterUI::renderState()
{
    if (!ImGui::TreeNode("State##CharacterUI::renderState"))
        return;

    ImGui::DragFloat("m_ragdolActivateFactor", &m_character->m_ragdolActivateFactor, 0.5f);

    // TODO: fps camera - change camera min pitch
    ImGui::Checkbox("m_aimLocked", &m_controller->m_aimLocked);
    ImGui::Checkbox("m_controlCharacter", &m_character->m_controlCharacter);
    ImGui::Checkbox("m_followCharacter", &m_character->m_followCharacter);
    ImGui::Checkbox("m_headFollow", &m_character->m_headFollow);

    ImGui::TreePop();
}

void CharacterUI::renderAttachments()
{
    if (!ImGui::TreeNode("State##CharacterUI::renderAttachments"))
        return;

    if (ImGui::TreeNode("Pistol"))
    {
        VectorUI::renderVec3("m_pistolOffset", m_character->m_pistolOffset, 0.1f);
        VectorUI::renderQuatEuler("m_pistolOrientation", m_character->m_pistolOrientation, 0.01f);
        ImGui::DragFloat("m_pistolScale", &m_character->m_pistolScale, 0.01f);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Muzzle"))
    {
        VectorUI::renderVec3("m_pistolOffset", m_character->m_muzzleOffset, 0.1f);
        ImGui::TreePop();
    }
    ImGui::DragFloat("m_fireLimit", &m_character->m_fireLimit, 0.05f);
    ImGui::DragFloat("m_fireAnimStartTime", &m_character->m_fireAnimStartTime, 0.05f);
    ImGui::DragInt("m_fireAnimTimeMilli", &m_character->m_fireAnimTimeMilli, 25);
    ImGui::DragFloat("m_aimStateChangeSpeed", &m_character->m_aimStateChangeSpeed, 0.1f);

    ImGui::TreePop();
}

void CharacterUI::renderController()
{
    if (!ImGui::TreeNode("Controller##CharacterUI::renderController"))
        return;

    renderSpeedLimiter(m_character->m_controller->m_walkSpeed, "m_walkSpeed");
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
    ImGui::DragFloat("m_minTurnFactor", &m_controller->m_minTurnFactor, 0.001f, 0.f, 1.f);
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
    ImGui::DragFloat("m_moveForce", &m_controller->m_moveForce, 1.f, 0);
    ImGui::DragFloat("m_runForce", &m_controller->m_runForce, 1.f, 0);
    ImGui::DragFloat("m_jumpForce", &m_controller->m_jumpForce, 0.1f, 0);
    ImGui::DragFloat("m_turnForce", &m_controller->m_turnForce, 0.001f, 0);
    ImGui::DragFloat("m_toIdleForce", &m_controller->m_toIdleForce, 0.1f, 0);
    ImGui::DragFloat("m_toIdleForceHoriz", &m_controller->m_toIdleForceHoriz, 0.1f, 0);
    ImGui::DragFloat("m_groundThreshold", &m_controller->m_groundThreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnThreshold", &m_controller->m_turnThreshold, 0.001f, 0);
    ImGui::DragFloat("m_walkToRunAnimThreshold", &m_controller->m_walkToRunAnimThreshold, 0.1f, 0);
    ImGui::DragFloat("m_turnAnimForce", &m_controller->m_turnAnimForce, 0.01f, 0);
    ImGui::DragFloat("m_turnAnimMaxFactor", &m_controller->m_turnAnimMaxFactor, 0.1f, 0);
    ImGui::DragFloat("m_floatElevation", &m_controller->m_floatElevation, 0.001f, 0);

    ImGui::TreePop();
}

void CharacterUI::renderMovement()
{
    if (!ImGui::TreeNode("Movement##CharacterUI::renderMovement"))
        return;

    ImGui::DragFloat("m_verticalSpeed", &m_controller->m_verticalSpeed, 0.1f);
    ImGui::Text("Move Stage");
    ImGui::Text("idle: %.1f", m_character->m_moveStage.idle);
    ImGui::Text("walk: %.1f", m_character->m_moveStage.walk);
    ImGui::Text("run: %.1f", m_character->m_moveStage.run);
    ImGui::DragFloat("idleEndGap", &m_character->m_moveStage.idleEndGap, 0.1f);
    ImGui::DragFloat("walkStartGap", &m_character->m_moveStage.walkStartGap, 0.1f);
    ImGui::DragFloat("walkEndGap", &m_character->m_moveStage.walkEndGap, 0.1f);
    ImGui::DragFloat("runStartGap", &m_character->m_moveStage.runStartGap, 0.1f);
    ImGui::Separator();
    ImGui::Text("Move Orient");
    ImGui::Text("forward: %.1f", m_character->m_moveOrient.forward);
    ImGui::Text("back: %.1f", m_character->m_moveOrient.back);
    ImGui::Text("left: %.1f", m_character->m_moveOrient.left);
    ImGui::Text("right: %.1f", m_character->m_moveOrient.right);
    ImGui::Text("backLeft: %.1f", m_character->m_moveOrient.backLeft);
    ImGui::Text("backRight: %.1f", m_character->m_moveOrient.backRight);
    ImGui::Separator();

    ImGui::TreePop();
}

void CharacterUI::renderPhysics()
{
    if (!ImGui::TreeNode("Physics##CharacterUI::renderPhysics"))
        return;

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
    float angularDamping = m_rb->getAngularDamping();
    if (ImGui::DragFloat("linearDamping", &linearDamping, 0.1))
    {
        m_rb->setDamping(linearDamping, angularDamping);
    }
    if (ImGui::DragFloat("angularDamping", &angularDamping, 0.1))
    {
        m_rb->setDamping(linearDamping, angularDamping);
    }

    ImGui::TreePop();
}

void CharacterUI::renderSpeedLimiter(SpeedLimiter &speedLimiter, std::string name)
{
    if (!ImGui::TreeNode((name + "##CharacterUI::renderSpeedLimiter").c_str()))
        return;

    const int numPoints = 100;
    float values[numPoints];
    const float rangeMin = -M_PI;
    const float rangeMax = M_PI;

    ImGui::DragFloat((name + "::m_constantValue").c_str(), &speedLimiter.m_constantValue, 0.01f);
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

    ImGui::TreePop();
}
