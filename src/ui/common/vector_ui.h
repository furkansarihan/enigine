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
    static bool renderVec2(const char *header, glm::vec2 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);

        return modified;
    }

    static bool renderVec3(const char *header, glm::vec3 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &vec.z, dragSpeed);

        return modified;
    }

    static bool renderVec3(const char *header, btVector3 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), (float *)&vec.m_floats[0], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), (float *)&vec.m_floats[1], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), (float *)&vec.m_floats[2], dragSpeed);

        return modified;
    }

    static bool renderVec4(const char *header, glm::vec4 &vec, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &vec.x, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &vec.y, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &vec.z, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "A").c_str(), &vec.a, dragSpeed);

        return modified;
    }

    static bool renderQuat(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "W").c_str(), &quat[0], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &quat[1], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &quat[2], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &quat[3], dragSpeed);

        return modified;
    }

    static bool renderQuatEuler(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        glm::vec3 euler = glm::eulerAngles(quat);
        euler = glm::degrees(euler);

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &euler.x, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &euler.y, dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &euler.z, dragSpeed);

        euler = glm::radians(euler);
        quat = glm::quat(euler);

        return modified;
    }

    static bool renderNormalizedQuat(const char *header, glm::quat &quat, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "W").c_str(), &quat[0], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &quat[1], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &quat[2], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &quat[3], dragSpeed);

        quat = glm::normalize(quat);

        return modified;
    }

    static bool renderNormalizedQuat(const char *header, glm::vec4 &vector, float dragSpeed)
    {
        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return false;

        bool modified = false;
        modified |= ImGui::DragFloat((std::string(header) + "X").c_str(), &vector[0], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Y").c_str(), &vector[1], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "Z").c_str(), &vector[2], dragSpeed);
        modified |= ImGui::DragFloat((std::string(header) + "W").c_str(), &vector[3], dragSpeed);

        glm::quat normalized = glm::normalize(glm::quat(vector.w, vector.x, vector.y, vector.z));
        vector.x = normalized.x;
        vector.y = normalized.y;
        vector.z = normalized.z;
        vector.w = normalized.w;

        return modified;
    }

    static bool renderTransform(const char *header, eTransform &transform, float dragPos, float dragRot, float dragScale)
    {
        bool changed = false;

        if (!ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_NoTreePushOnOpen))
            return changed;

        glm::vec3 pos = transform.getPosition();
        glm::vec3 posCopy = pos;
        VectorUI::renderVec3((std::string(header) + ":pos").c_str(), pos, dragPos);
        if (pos != posCopy)
        {
            transform.setPosition(pos);
            changed = true;
        }
        ImGui::Separator();

        glm::quat rot = transform.getRotation();
        glm::quat rotCopy = rot;
        VectorUI::renderNormalizedQuat((std::string(header) + ":rot").c_str(), rot, dragRot);
        if (rot != rotCopy)
        {
            transform.setRotation(rot);
            changed = true;
        }
        ImGui::Separator();

        glm::vec3 scale = transform.getScale();
        glm::vec3 scaleCopy = scale;
        VectorUI::renderVec3((std::string(header) + ":scale").c_str(), scale, dragScale);
        if (scale != scaleCopy)
        {
            transform.setScale(scale);
            changed = true;
        }
        ImGui::Separator();

        return changed;
    }
};

#endif /* vector_ui_hpp */
