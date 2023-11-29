#include <vector>

#include "animator.h"

Animator::Animator(std::vector<Animation *> animations)
    : m_animations(animations)
{
    // TODO: initial state

    // TODO: better way?
    int size = m_animations[0]->m_bones.size();
    m_finalBoneMatrices.reserve(size);
    for (int i = 0; i < size; i++)
        m_finalBoneMatrices.push_back(glm::mat4(1.0f));

    m_globalMatrices.reserve(size);
    for (int i = 0; i < size; i++)
        m_globalMatrices.push_back(glm::mat4(1.0f));
}

Animator::~Animator()
{
    for (int i = 0; i < m_state.animations.size(); i++)
        delete m_state.animations[i];
    m_state.animations.clear();

    for (int i = 0; i < m_state.poses.size(); i++)
        delete m_state.poses[i];
    m_state.poses.clear();
}

void Animator::update(float deltaTime)
{
    for (int i = 0; i < m_state.animations.size(); i++)
        m_state.animations[i]->updateTimer(deltaTime, m_startOffset);

    for (int i = 0; i < m_state.poses.size(); i++)
        m_state.poses[i]->updateTimer(deltaTime, m_startOffset);

    calculateBoneTransform(m_animations[0]->m_rootNode, glm::mat4(1.0f));
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
        Anim *anim = m_state.animations[i];
        Bone *bone = anim->m_animation->getBone(nodeName);

        if (!bone)
            continue;

        float blendWeight = anim->m_blendFactor * bone->m_blendFactor;

        if (blendWeight == 0.0f)
            continue;

        bone->update(anim->m_timer);

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

    // pose influence
    for (int i = 0; i < m_state.poses.size(); i++)
    {
        Anim *anim = m_state.poses[i];
        Animation *animation = anim->m_animation;
        Bone *bone = animation->getBone(nodeName);

        if (!bone)
            continue;

        float blendWeight = anim->m_blendFactor * bone->m_blendFactor;

        if (blendWeight == 0.0f)
            continue;

        bone->update(anim->m_timer);

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
    auto boneInfoMap = m_animations[0]->m_boneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_finalBoneMatrices[index] = globalTransformation * offset;
        m_globalMatrices[index] = globalTransformation;
    }

    for (int i = 0; i < node->children.size(); i++)
        calculateBoneTransform(node->children[i], globalTransformation);
}

Anim *Animator::addStateAnimation(Animation *animation)
{
    Anim *anim = new Anim(animation);
    m_state.animations.push_back(anim);

    return anim;
}

Anim *Animator::addPoseAnimation(Animation *animation)
{
    Anim *anim = new Anim(animation);
    m_state.poses.push_back(anim);

    return anim;
}

// Anim

Anim::Anim(Animation *animation)
    : m_animation(animation),
      m_blendFactor(0.f),
      m_playbackSpeed(1.f),
      m_timerActive(true),
      m_timer(0.f)
{
}

void Anim::updateTimer(float deltaTime, float startOffset)
{
    if (!m_timerActive)
        return;

    // TODO: ?
    // if (m_blendFactor == 0.f)
    //     return;

    m_timer += m_animation->m_ticksPerSecond * m_playbackSpeed * deltaTime;

    float clampedTime = fmod(m_timer, m_animation->m_duration);
    if (clampedTime < m_timer)
        clampedTime += startOffset;

    m_timer = clampedTime;
}
