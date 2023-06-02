#include "character_ui.h"

void renderQuaternion(const char* label, glm::quat& quaternion);

void CharacterUI::render()
{
    if (!ImGui::CollapsingHeader("Character", ImGuiTreeNodeFlags_NoTreePushOnOpen))
        return;

    // TODO: fps camera - change camera min pitch
    ImGui::Checkbox("m_aimLocked", &m_controller->m_aimLocked);
    ImGui::Checkbox("m_controlCharacter", &m_character->m_controlCharacter);
    ImGui::Checkbox("m_followCharacter", &m_character->m_followCharacter);
    ImGui::Checkbox("m_drawBones", &m_drawBones);
    if (ImGui::CollapsingHeader("m_followOffsetNormal", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat("m_followOffsetNormalX", &m_character->m_followOffsetNormal.x, 0.1f);
        ImGui::DragFloat("m_followOffsetNormalY", &m_character->m_followOffsetNormal.y, 0.1f);
        ImGui::DragFloat("m_followOffsetNormalZ", &m_character->m_followOffsetNormal.z, 0.1f);
    }
    if (ImGui::CollapsingHeader("m_followOffsetAim", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat("m_followOffsetAimX", &m_character->m_followOffsetAim.x, 0.1f);
        ImGui::DragFloat("m_followOffsetAimY", &m_character->m_followOffsetAim.y, 0.1f);
        ImGui::DragFloat("m_followOffsetAimZ", &m_character->m_followOffsetAim.z, 0.1f);
    }
    if (ImGui::CollapsingHeader("Pistol", ImGuiTreeNodeFlags_NoTreePushOnOpen))
    {
        ImGui::DragFloat("m_pistolOffsetX", &m_character->m_pistolOffset.x, 0.1f);
        ImGui::DragFloat("m_pistolOffsetY", &m_character->m_pistolOffset.y, 0.1f);
        ImGui::DragFloat("m_pistolOffsetZ", &m_character->m_pistolOffset.z, 0.1f);
        renderQuaternion("m_pistolOrientation", m_character->m_pistolOrientation);
        ImGui::DragFloat("m_boneScale", &m_boneScale, 0.01f);
        ImGui::DragFloat("m_pistolScale", &m_character->m_pistolScale, 0.01f);
    }
    ImGui::DragFloat("m_aimStateChangeSpeed", &m_character->m_aimStateChangeSpeed, 0.1f);
    ImGui::DragFloat("m_followOffsetFactor", &m_character->m_followOffsetFactor, 0.1f);
    ImGui::DragFloat("m_leftBlendEdge", &m_character->m_leftBlendEdge, 0.01f);
    ImGui::DragFloat("m_rightBlendEdge", &m_character->m_rightBlendEdge, 0.01f);
    ImGui::DragFloat("m_leftForward", &m_character->m_leftForward, 0.01f);
    ImGui::DragFloat("m_rightForward", &m_character->m_rightForward, 0.01f);
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

void renderQuaternion(const char* label, glm::quat& quaternion)
{
    float eulerX = glm::degrees(glm::pitch(quaternion));
    float eulerY = glm::degrees(glm::yaw(quaternion));
    float eulerZ = glm::degrees(glm::roll(quaternion));

    if (ImGui::SliderFloat((std::string(label) + "_X").c_str(), &eulerX, -180.0f, 180.0f))
        quaternion = glm::quat(glm::radians(glm::vec3(eulerX, eulerY, eulerZ)));

    if (ImGui::SliderFloat((std::string(label) + "_Y").c_str(), &eulerY, -180.0f, 180.0f))
        quaternion = glm::quat(glm::radians(glm::vec3(eulerX, eulerY, eulerZ)));

    if (ImGui::SliderFloat((std::string(label) + "_Z").c_str(), &eulerZ, -180.0f, 180.0f))
        quaternion = glm::quat(glm::radians(glm::vec3(eulerX, eulerY, eulerZ)));

    ImGui::Text("Quaternion: (%.3f, %.3f, %.3f, %.3f)", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
}

void CharacterUI::drawArmatureBones(Character &character, Shader &simpleShader, Model &cube, glm::mat4 viewProjection)
{
    if (!m_drawBones)
        return;

    simpleShader.use();
    simpleShader.setVec4("DiffuseColor", glm::vec4(1.0, 0.0, 1.0, 1.0f));
    for (int i = 0; i < m_bones.size(); i++)
    {
        int index = character.m_animator->m_animations[0]->m_BoneInfoMap[m_bones[i]].id;
        glm::mat4 model = character.m_animator->m_globalMatrices[index];

        glm::mat4 model2(1.0f);
        model2 = glm::translate(model2, character.m_position);
        model2 = glm::rotate(model2, character.m_rotation.y * (1.0f - character.getRagdolPose().blendFactor), glm::vec3(0, 1, 0));
        model2 = glm::scale(model2, glm::vec3(character.m_scale));

        model2 = model2 * model;

        model2 = glm::scale(model2, glm::vec3(m_boneScale));

        simpleShader.setMat4("MVP", viewProjection * model2);
        cube.draw(simpleShader);
    }
}
