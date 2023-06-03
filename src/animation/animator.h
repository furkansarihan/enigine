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

struct Anim
{
    int index;
    float blendFactor;
};

struct AnimPose
{
    int index;
    float blendFactor;
};

struct AnimatorState
{
    std::vector<Anim> animations;
    std::vector<AnimPose> poses;
};

class Animator
{
public:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    std::vector<glm::mat4> m_globalMatrices;
    std::vector<Animation *> m_animations;
    std::vector<float> m_timers;
    AnimatorState m_state;

    Animator(std::vector<Animation *> animations);
    ~Animator();
    void update(float deltaTime);
    void calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform);
    void setAnimTime(int animIndex, float time);
};

#endif /* animator_hpp */
