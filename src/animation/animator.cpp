#include <vector>

#include "animator.h"

Animator::Animator(Animation *animation)
{
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    // TODO: better way?
    m_FinalBoneMatrices.reserve(MAX_BONES);
    for (int i = 0; i < MAX_BONES; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

Animator::~Animator()
{
    // TODO: destruction
}

void Animator::update(float deltaTime)
{
    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * deltaTime;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
        calculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
    }

    // std::cout << "m_CurrentTime: " << m_CurrentTime << std::endl;
}

void Animator::play(Animation *pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *bone = m_CurrentAnimation->FindBone(nodeName);

    if (bone)
    {
        bone->update(m_CurrentTime);
        nodeTransform = bone->m_LocalTransform;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}
