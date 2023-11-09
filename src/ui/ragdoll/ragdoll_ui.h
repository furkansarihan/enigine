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
    float m_floatHeight = 3.f;
    bool m_activateObject = true;
    int m_floatIndex = 2;

    void render() override;
    void renderConeTwist(int index, btConeTwistConstraint *constraint);
    void renderHinge(int index, btHingeConstraint *constraint);
    std::string getJointName(int index);
};

#endif /* ragdoll_ui_hpp */
