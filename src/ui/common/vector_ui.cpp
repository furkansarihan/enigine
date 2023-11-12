#include "vector_ui.h"

#include <cstring>

std::string extractStringBeforeDoubleHash(const std::string &input)
{
    size_t pos = input.find("##");
    if (pos != std::string::npos)
        return input.substr(0, pos);
    else
        return input;
}

bool VectorUI::renderVec2(const char *header, glm::vec2 &vec, float dragSpeed)
{
    return ImGui::DragFloat2(header, &vec.x, dragSpeed);
}

bool VectorUI::renderVec3(const char *header, glm::vec3 &vec, float dragSpeed)
{
    return ImGui::DragFloat3(header, &vec.x, dragSpeed);
}

bool VectorUI::renderVec3(const char *header, btVector3 &vec, float dragSpeed)
{
    return ImGui::DragFloat3(header, (float *)&vec.m_floats[0], dragSpeed);
}

bool VectorUI::renderVec4(const char *header, glm::vec4 &vec, float dragSpeed)
{
    return ImGui::DragFloat4(header, &vec.x, dragSpeed);
}

bool VectorUI::renderQuat(const char *header, glm::quat &quat, float dragSpeed)
{
    return ImGui::DragFloat4(header, &quat.w, dragSpeed);
}

bool VectorUI::renderQuat(const char *header, btQuaternion &quat, float dragSpeed)
{
    glm::quat glmQuat = BulletGLM::getGLMQuat(quat);

    bool modified = ImGui::DragFloat4(header, &glmQuat.w, dragSpeed);

    glmQuat = glm::normalize(glmQuat);

    if (modified)
        quat = BulletGLM::getBulletQuat(glmQuat);

    return modified;
}

bool VectorUI::renderQuatEuler(const char *header, btQuaternion &quat, float dragSpeed)
{
    glm::quat glmQuat = BulletGLM::getGLMQuat(quat);

    glm::vec3 euler = glm::eulerAngles(glmQuat);
    euler = glm::degrees(euler);

    bool modified = ImGui::DragFloat3(header, &euler.x, dragSpeed);

    euler = glm::radians(euler);
    glmQuat = glm::quat(euler);

    glmQuat = glm::normalize(glmQuat);

    if (modified)
        quat = BulletGLM::getBulletQuat(glmQuat);

    return modified;
}

bool VectorUI::renderQuatEuler(const char *header, glm::quat &quat, float dragSpeed)
{
    glm::vec3 euler = glm::eulerAngles(quat);
    euler = glm::degrees(euler);

    bool modified = ImGui::DragFloat3(header, &euler.x, dragSpeed);

    euler = glm::radians(euler);
    quat = glm::quat(euler);

    return modified;
}

bool VectorUI::renderNormalizedQuat(const char *header, glm::quat &quat, float dragSpeed)
{
    bool modified = ImGui::DragFloat4(header, &quat.w, dragSpeed);

    if (modified)
        quat = glm::normalize(quat);

    return modified;
}

bool VectorUI::renderNormalizedQuat(const char *header, glm::vec4 &vector, float dragSpeed)
{
    bool modified = ImGui::DragFloat4(header, &vector.w, dragSpeed);

    if (modified)
    {
        glm::quat normalized = glm::normalize(glm::quat(vector.w, vector.x, vector.y, vector.z));
        vector.x = normalized.x;
        vector.y = normalized.y;
        vector.z = normalized.z;
        vector.w = normalized.w;
    }

    return modified;
}

bool VectorUI::renderTransform(const char *header, eTransform &transform, float dragPos, float dragRot, float dragScale)
{
    bool modified = false;

    ImGui::Text("%s", extractStringBeforeDoubleHash(std::string(header)).c_str());
    glm::vec3 pos = transform.getPosition();
    if (VectorUI::renderVec3(("position##" + std::string(header)).c_str(), pos, dragPos))
    {
        transform.setPosition(pos);
        modified = true;
    }
    glm::quat rot = transform.getRotation();
    if (VectorUI::renderNormalizedQuat(("rotation##" + std::string(header)).c_str(), rot, dragRot))
    {
        transform.setRotation(rot);
        modified = true;
    }
    glm::vec3 scale = transform.getScale();
    if (VectorUI::renderVec3(("scale##" + std::string(header)).c_str(), scale, dragScale))
    {
        transform.setScale(scale);
        modified = true;
    }

    return modified;
}

bool VectorUI::renderTransform(const char *header, btTransform &transform, float dragPos, float dragRot, float dragScale)
{
    bool modified = false;

    ImGui::Text("%s", extractStringBeforeDoubleHash(std::string(header)).c_str());

    btVector3 pos = transform.getOrigin();
    if (VectorUI::renderVec3(("position##" + std::string(header)).c_str(), pos, dragPos))
    {
        transform.setOrigin(pos);
        modified = true;
    }

    btQuaternion rot = transform.getRotation();
    if (VectorUI::renderQuat(("rotation##" + std::string(header)).c_str(), rot, dragRot))
    {
        transform.setRotation(rot);
        modified = true;
    }

    // TODO: scale

    return modified;
}
