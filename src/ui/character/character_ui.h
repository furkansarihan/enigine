#ifndef character_ui_hpp
#define character_ui_hpp

#include "../base_ui.h"
#include "../../camera/camera.h"
#include "../../character_controller/character_controller.h"

class CharacterUI : public BaseUI
{
private:
    CharacterController *m_controller;
    btRigidBody *m_rb;

public:
    CharacterUI(CharacterController *controller, btRigidBody *rb) : m_controller(controller), m_rb(rb) {}

    void render() override;
};

#endif /* character_ui_hpp */
