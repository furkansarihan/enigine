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
    }

    bool m_renderLastEnterPath = true;

    void render() override;
    void renderSpeedLimiter(SpeedLimiter &speedLimiter, std::string name);
    void renderLastEnterCarPath();
    void drawArmatureBones(Character &character, Shader &simpleShader, Model &model, glm::mat4 viewProjection);
};

#endif /* character_ui_hpp */
