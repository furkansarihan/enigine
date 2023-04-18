#include <vector>

#include "animation.h"

Animation::Animation(const std::string &animationPath, Model *model)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    readHierarchy(m_RootNode, scene->mRootNode);
    readBones(animation, *model);
}

Animation::~Animation()
{
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

void Animation::readHierarchy(AssimpNodeData &dest, const aiNode *src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = AssimpToGLM::getGLMMat4(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        readHierarchy(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
