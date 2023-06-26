#include <vector>

#include "animator.h"

Animator::Animator(std::vector<Animation *> animations)
{
    m_animations = animations;
    for (int i = 0; i < m_animations.size(); i++)
        m_timers.push_back(0.0f);

    // TODO: initial state

    // TODO: better way?
    m_finalBoneMatrices.reserve(MAX_BONES);
    for (int i = 0; i < MAX_BONES; i++)
        m_finalBoneMatrices.push_back(glm::mat4(1.0f));

    m_globalMatrices.reserve(MAX_BONES);
    for (int i = 0; i < MAX_BONES; i++)
        m_globalMatrices.push_back(glm::mat4(1.0f));
}

Animator::~Animator()
{
    // TODO: destruction
}

// TODO: move validations to setters
void Animator::update(float deltaTime)
{
    for (int i = 0; i < m_animations.size(); i++)
    {
        m_timers[i] += m_animations[i]->m_TicksPerSecond * m_animations[i]->m_playbackSpeed * deltaTime;

        float clampedTime = fmod(m_timers[i], m_animations[i]->m_Duration);
        if (clampedTime < m_timers[i])
            clampedTime += m_startOffset;

        m_timers[i] = clampedTime;
    }

    calculateBoneTransform(&m_animations[0]->m_RootNode, glm::mat4(1.0f));
}

void Animator::calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    glm::vec3 blendedT;
    glm::quat blendedR;
    glm::vec3 blendedS;

    bool boneProcessed = false;
    float totalWeight = 0.0f;
    for (int i = 0; i < m_state.animations.size(); i++)
    {
        Anim anim = m_state.animations[i];
        Bone *bone = m_animations[anim.index]->getBone(nodeName);
        float blendWeight = anim.blendFactor * bone->m_blendFactor;

        if (!bone)
            continue;
        if (blendWeight == 0.0f)
            continue;

        bone->update(m_timers[anim.index]);

        totalWeight += blendWeight;

        // TODO: blend weight influence
        if (!boneProcessed) // first bone
        {
            blendedT = bone->m_translation;
            blendedR = bone->m_rotation;
            blendedS = bone->m_scale;
        }
        else
        {
            float weight = blendWeight / totalWeight;
            if (isnan(weight))
                weight = 1.0f;

            blendedT = glm::mix(blendedT, bone->m_translation, weight);
            blendedR = glm::slerp(blendedR, bone->m_rotation, weight);
            blendedS = glm::mix(blendedS, bone->m_scale, weight);
        }

        boneProcessed = true;
    }

    // AnimPose influence
    for (int i = 0; i < m_state.poses.size(); i++)
    {
        AnimPose animPose = m_state.poses[i];
        Animation *animation = m_animations[animPose.index];
        Bone *bone = animation->getBone(nodeName);
        float blendWeight = animPose.blendFactor * bone->m_blendFactor;

        if (!bone)
            continue;
        if (blendWeight == 0.0f)
            continue;

        if (animation->m_timed)
            bone->update(animPose.time);
        else
            bone->update(m_timers[animPose.index]);

        blendedT = glm::mix(blendedT, bone->m_translation, blendWeight);
        blendedR = glm::slerp(blendedR, bone->m_rotation, blendWeight);
        blendedS = glm::mix(blendedS, bone->m_scale, blendWeight);
    }

    if (boneProcessed)
    {
        nodeTransform = glm::translate(glm::mat4(1), blendedT) *
                        glm::toMat4(glm::normalize(blendedR)) *
                        glm::scale(glm::mat4(1), blendedS);
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    // TODO: always first index is correct?
    auto boneInfoMap = m_animations[0]->m_BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_finalBoneMatrices[index] = globalTransformation * offset;
        m_globalMatrices[index] = globalTransformation;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}

void Animator::setAnimTime(int animIndex, float time)
{
    m_timers[animIndex] = time;
}
