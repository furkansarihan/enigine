#ifndef animation_ui_hpp
#define animation_ui_hpp

#include "../base_ui.h"
#include "../../animation/animation.h"
#include "../../animation/animator.h"

class AnimationUI : public BaseUI
{
private:
    Animator *m_animator;

public:
    AnimationUI(Animator *animator);

    int m_selectedAnimPose;

    void render() override;

    void renderBlendTable();
    void renderSelectedAnimPose();
};

#endif /* animation_ui_hpp */
