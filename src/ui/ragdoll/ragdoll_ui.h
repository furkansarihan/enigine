#ifndef ragdoll_ui_hpp
#define ragdoll_ui_hpp

#include "../base_ui.h"
#include "../../character/character.h"
#include "../../ragdoll/ragdoll.h"

class RagdollUI : public BaseUI
{
private:
    Character *m_character;
    Camera *m_camera;

public:
    RagdollUI(Character *character, Camera *camera)
        : m_character(character),
          m_camera(camera)
    {
    }

    bool m_floatObject = false;
    float m_floatHeight = 10.f;
    bool m_activateObject = true;
    int m_floatIndex = 2;

    void render() override;
};

#endif /* ragdoll_ui_hpp */
