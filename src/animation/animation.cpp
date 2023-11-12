#include <vector>

#include "animation.h"

Animation::Animation(const std::string &animationName, Model *model, bool timed)
    : m_name(animationName),
      m_timed(timed)
{
    const aiScene *scene = model->m_scene;
    assert(scene && scene->mRootNode);
    // std::cout << "Animation: mNumAnimations: " << scene->mNumAnimations << std::endl;

    aiAnimation *animation;
    for (int i = 0; i < scene->mNumAnimations; i++)
    {
        auto anim = scene->mAnimations[i];
        if (anim && strcmp(anim->mName.C_Str(), animationName.c_str()) == 0)
        {
            animation = anim;
            break;
        }
    }

    if (animation == nullptr)
    {
        std::cout << "Animation: can not found an animation with name: " << animationName << std::endl;
        return;
    }

    // std::cout << "Animation: mName: " << animation->mName.C_Str() << std::endl;
    // std::cout << "Animation: mDuration: " << animation->mDuration << std::endl;
    // std::cout << "Animation: mTicksPerSecond: " << animation->mTicksPerSecond << std::endl;
    // std::cout << "Animation: mNumChannels: " << animation->mNumChannels << std::endl
    //           << std::endl;

    m_RootNode = new AssimpNodeData();

    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    readHierarchy(m_RootNode, scene->mRootNode);
    readBones(animation, *model);
}

Animation::~Animation()
{
    for (auto iter = m_assimpNodes.begin(); iter != m_assimpNodes.end(); ++iter)
    {
        delete iter->second;
    }
    m_assimpNodes.clear();

    for (auto iter = m_bones.begin(); iter != m_bones.end(); ++iter)
    {
        delete iter->second;
    }
    m_bones.clear();

    // TODO: destruction
}

Bone *Animation::getBone(const std::string &name)
{
    if (m_bones.find(name) != m_bones.end())
    {
        return m_bones[name];
    }

    return nullptr;
}

void Animation::readBones(const aiAnimation *animation, Model &model)
{
    int size = animation->mNumChannels;

    auto &boneInfoMap = model.m_boneInfoMap;
    int &boneCount = model.m_boneCounter;

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        // std::cout << "readBones: mNumPositionKeys: " << channel->mNodeName.C_Str() << ": " << channel->mNumPositionKeys << std::endl;
        // std::cout << "readBones: mNumRotationKeys: " << channel->mNodeName.C_Str() << ": " << channel->mNumRotationKeys << std::endl;
        // std::cout << "readBones: mNumScalingKeys: " << channel->mNodeName.C_Str() << ": " << channel->mNumScalingKeys << std::endl;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            std::cout << "Animation: readBones: missing bone found" << std::endl;
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }

        Bone *bone = new Bone(boneName, boneInfoMap[boneName].id, channel);
        m_bones[boneName] = bone;
    }

    m_BoneInfoMap = boneInfoMap;
}

// TODO: only read bones?
void Animation::readHierarchy(AssimpNodeData *dest, const aiNode *src)
{
    assert(src);

    dest->name = src->mName.data;
    dest->transformation = AssimpToGLM::getGLMMat4(src->mTransformation);
    m_assimpNodes[dest->name] = dest;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData *newData = new AssimpNodeData();

        readHierarchy(newData, src->mChildren[i]);
        dest->children.push_back(newData);
    }
}

void Animation::setBlendMask(std::unordered_map<std::string, float> blendMask, float defaultValue)
{
    m_blendMask = blendMask;

    for (auto it = m_bones.begin(); it != m_bones.end(); ++it)
    {
        Bone *bone = it->second;
        bone->m_blendFactor = defaultValue;
    }

    for (auto it = m_blendMask.begin(); it != m_blendMask.end(); ++it)
    {
        std::string name = it->first;
        Bone *bone = getBone(name);
        if (bone)
        {
            float blendFactor = it->second;
            bone->m_blendFactor = blendFactor;
        }
    }
}
