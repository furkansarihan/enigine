#ifndef ragdoll_ui_hpp
#define ragdoll_ui_hpp

#include "../base_ui.h"
#include "../../character_controller/character_controller.h"
#include "../../ragdoll/ragdoll.h"

class RagdollUI : public BaseUI
{
private:
    Ragdoll *m_ragdoll;
    CharacterController *m_characterController;
    Camera *m_camera;

public:
    RagdollUI(Ragdoll *ragdoll, CharacterController *characterController, Camera *camera)
        : m_ragdoll(ragdoll),
          m_characterController(characterController),
          m_camera(camera)
    {
    }

    bool m_floatObject = false;
    bool m_activateObject = true;
    int m_floatIndex = 2;

    void render() override;
};

#endif /* ragdoll_ui_hpp */
