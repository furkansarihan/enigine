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

    Animation *m_selectedAnimation;

    void render() override;

    void renderBlendTable(const std::string &tableName, std::vector<Anim *> &anims);
    void renderSelectedAnimation();
};

#endif /* animation_ui_hpp */
