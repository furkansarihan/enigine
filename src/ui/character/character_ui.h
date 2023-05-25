#ifndef character_ui_hpp
#define character_ui_hpp

#include "../base_ui.h"
#include "../../camera/camera.h"
#include "../../character_controller/character_controller.h"
#include "../../character/playable_character.h"

class CharacterUI : public BaseUI
{
private:
    PCharacter *m_character;
    CharacterController *m_controller;
    btRigidBody *m_rb;

public:
    CharacterUI(PCharacter *character, CharacterController *controller, btRigidBody *rb)
        : m_character(character),
          m_controller(controller),
          m_rb(rb)

    {
    }

    void render() override;
};

#endif /* character_ui_hpp */
