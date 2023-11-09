#ifndef character_ui_hpp
#define character_ui_hpp

#include "../base_ui.h"
#include "../../camera/camera.h"
#include "../../character/character.h"
#include "../../shader/shader.h"
#include "../../model/model.h"
#include "../../character_controller/character_controller.h"
#include "../../character/playable_character.h"
#include "../../utils/common.h"

class CharacterUI : public BaseUI
{
private:
    PCharacter *m_character;
    CharacterController *m_controller;
    btRigidBody *m_rb;

public:
    CharacterUI(PCharacter *character)
        : m_character(character),
          m_controller(character->m_controller),
          m_rb(character->m_rigidbody)

    {
        m_bones.push_back("mixamorig:Hips");
        m_bones.push_back("mixamorig:Spine");
        m_bones.push_back("mixamorig:Spine1");
        m_bones.push_back("mixamorig:Spine2");
        m_bones.push_back("mixamorig:Neck");
        m_bones.push_back("mixamorig:Head");
        m_bones.push_back("mixamorig:LeftShoulder");
        m_bones.push_back("mixamorig:RightShoulder");
        m_bones.push_back("mixamorig:LeftArm");
        m_bones.push_back("mixamorig:RightArm");
        m_bones.push_back("mixamorig:LeftForeArm");
        m_bones.push_back("mixamorig:RightForeArm");
        m_bones.push_back("mixamorig:LeftHand");
        m_bones.push_back("mixamorig:RightHand");
        m_bones.push_back("mixamorig:RightUpLeg");
        m_bones.push_back("mixamorig:RightLeg");
        m_bones.push_back("mixamorig:RightFoot");
        m_bones.push_back("mixamorig:RightToeBase");
        m_bones.push_back("mixamorig:LeftUpLeg");
        m_bones.push_back("mixamorig:LeftLeg");
        m_bones.push_back("mixamorig:LeftFoot");
        m_bones.push_back("mixamorig:LeftToeBase");
    }

    std::vector<std::string> m_bones;
    float m_boneScale = 5.f;
    bool m_drawBones = false;
    bool m_renderLastEnterPath = true;

    void render() override;
    void renderSpeedLimiter(SpeedLimiter &speedLimiter, std::string name);
    void renderLastEnterCarPath();
    void drawArmatureBones(Character &character, Shader &simpleShader, Model &model, glm::mat4 viewProjection);
};

#endif /* character_ui_hpp */
