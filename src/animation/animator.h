#ifndef animator_hpp
#define animator_hpp

#include <vector>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include "animation.h"

#define MAX_BONES 200

class Anim
{
public:
    Anim(Animation *animation);

    Animation *m_animation;
    float m_blendFactor;
    float m_playbackSpeed;
    bool m_timerActive;
    float m_timer;

    void updateTimer(float deltaTime, float startOffset);
};

struct AnimatorState
{
    std::vector<Anim *> animations;
    std::vector<Anim *> poses;
};

class Animator
{
public:
    std::vector<glm::mat4> m_finalBoneMatrices;
    std::vector<glm::mat4> m_globalMatrices;
    std::vector<Animation *> m_animations;
    AnimatorState m_state;
    float m_startOffset = 0.f;

    Animator(std::vector<Animation *> animations);
    ~Animator();
    void update(float deltaTime);
    void calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform);
    Anim *addStateAnimation(Animation *animation);
    Anim *addPoseAnimation(Animation *animation);
};

#endif /* animator_hpp */
