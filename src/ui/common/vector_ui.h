#ifndef vector_ui_hpp
#define vector_ui_hpp

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "btBulletDynamicsCommon.h"

#include "../../external/imgui/imgui.h"
#include "../../transform/transform.h"

class VectorUI
{
public:
    static void renderVec2(const char *header, glm::vec2 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;
        ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);
    }

    static void renderVec3(const char *header, glm::vec3 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;
        ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &vec.z, dragSpeed);
    }

    static void renderVec3(const char *header, btVector3 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;
        ImGui::DragFloat((std::string(header) + "X").c_str(), (float *)&vec.m_floats[0], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), (float *)&vec.m_floats[1], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), (float *)&vec.m_floats[2], dragSpeed);
    }

    static void renderQuat(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;
        ImGui::DragFloat((std::string(header) + "W").c_str(), &quat[0], dragSpeed);
        ImGui::DragFloat((std::string(header) + "X").c_str(), &quat[1], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &quat[2], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &quat[3], dragSpeed);
    }

    static void renderQuatEuler(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;

        glm::vec3 euler = glm::eulerAngles(quat);
        euler = glm::degrees(euler);

        ImGui::DragFloat((std::string(header) + "X").c_str(), &euler.x, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &euler.y, dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &euler.z, dragSpeed);

        euler = glm::radians(euler);
        quat = glm::quat(euler);
    }

    static void renderNormalizedQuat(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;
        ImGui::DragFloat((std::string(header) + "W").c_str(), &quat[0], dragSpeed);
        ImGui::DragFloat((std::string(header) + "X").c_str(), &quat[1], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Y").c_str(), &quat[2], dragSpeed);
        ImGui::DragFloat((std::string(header) + "Z").c_str(), &quat[3], dragSpeed);

        quat = glm::normalize(quat);
    }

    static void renderNormalizedQuat(const char *header, glm::vec4 &vector, float dragSpeed)
    {
        if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
        {
            ImGui::DragFloat((std::string(header) + "X").c_str(), &vector[0], dragSpeed);
            ImGui::DragFloat((std::string(header) + "Y").c_str(), &vector[1], dragSpeed);
            ImGui::DragFloat((std::string(header) + "Z").c_str(), &vector[2], dragSpeed);
            ImGui::DragFloat((std::string(header) + "W").c_str(), &vector[3], dragSpeed);
        }

        glm::quat normalized = glm::normalize(glm::quat(vector.w, vector.x, vector.y, vector.z));
        vector.x = normalized.x;
        vector.y = normalized.y;
        vector.z = normalized.z;
        vector.w = normalized.w;
    }

    static void renderTransform(const char *header, eTransform &transform, float dragPos, float dragRot, float dragScale)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return;

        glm::vec3 pos = transform.getPosition();
        glm::vec3 posCopy = pos;
        VectorUI::renderVec3((std::string(header) + ":pos").c_str(), pos, dragPos);
        if (pos != posCopy)
            transform.setPosition(pos);
        ImGui::Separator();

        glm::quat rot = transform.getRotation();
        glm::quat rotCopy = rot;
        VectorUI::renderNormalizedQuat((std::string(header) + ":rot").c_str(), rot, dragRot);
        if (rot != rotCopy)
            transform.setRotation(rot);
        ImGui::Separator();

        glm::vec3 scale = transform.getScale();
        glm::vec3 scaleCopy = scale;
        VectorUI::renderVec3((std::string(header) + ":scale").c_str(), scale, dragScale);
        if (scale != scaleCopy)
            transform.setScale(scale);
        ImGui::Separator();
    }
};

#endif /* vector_ui_hpp */
