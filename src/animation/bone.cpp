#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include "bone.h"

Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel)
    : m_Name(name),
      m_ID(ID),
      m_translation(1.0f),
      m_rotation(glm::vec3(1.0f)),
      m_scale(1.0f)
{
    m_NumPositions = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
    {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        KeyPosition data;
        data.timestamp = channel->mPositionKeys[positionIndex].mTime;
        data.position = AssimpToGLM::getGLMVec3(aiPosition);
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        KeyRotation data;
        data.timestamp = channel->mRotationKeys[rotationIndex].mTime;
        data.orientation = AssimpToGLM::getGLMQuat(aiOrientation);
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
    {
        aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
        KeyScale data;
        data.timestamp = channel->mScalingKeys[keyIndex].mTime;
        data.scale = AssimpToGLM::getGLMVec3(scale);
        m_Scales.push_back(data);
    }

    if (m_NumPositions == 1 && m_NumRotations == 1 && m_NumScalings == 1)
    {
        m_animType = AnimationType::Pose;

        m_translation = m_Positions[0].position;
        m_rotation = m_Rotations[0].orientation;
        m_scale = m_Scales[0].scale;
    }
    else
    {
        // TODO: validation
        m_animType = AnimationType::Cycle;
    }
}

Bone::~Bone()
{
    // TODO: destruction
}

// TODO: snap to keyframe
void Bone::update(float animationTime)
{
    if (m_animType == AnimationType::Pose)
        return;

    updateInternal(animationTime);
}

void Bone::updateInternal(float animationTime)
{
    m_translation = interpolatePosition(animationTime);
    m_rotation = interpolateRotation(animationTime);
    m_scale = interpolateScaling(animationTime);
}

int Bone::getPositionIndex(float animationTime)
{
    for (int index = 0; index < m_NumPositions - 1; ++index)
    {
        if (animationTime < m_Positions[index + 1].timestamp)
            return index;
    }
    assert(0);
}

int Bone::getRotationIndex(float animationTime)
{
    for (int index = 0; index < m_NumRotations - 1; ++index)
    {
        if (animationTime < m_Rotations[index + 1].timestamp)
            return index;
    }
    assert(0);
}

int Bone::getScaleIndex(float animationTime)
{
    for (int index = 0; index < m_NumScalings - 1; ++index)
    {
        if (animationTime < m_Scales[index + 1].timestamp)
            return index;
    }
    assert(0);
}

float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    scaleFactor = midWayLength / framesDiff;
    return scaleFactor;
}

glm::vec3 Bone::interpolatePosition(float animationTime)
{
    if (1 == m_NumPositions)
        return m_Positions[0].position;

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Positions[p0Index].timestamp,
                                       m_Positions[p1Index].timestamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
                                       m_Positions[p1Index].position, scaleFactor);
    return finalPosition;
}

glm::quat Bone::interpolateRotation(float animationTime)
{
    if (1 == m_NumRotations)
    {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Rotations[p0Index].timestamp,
                                       m_Rotations[p1Index].timestamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
                                         m_Rotations[p1Index].orientation, scaleFactor);
    return glm::normalize(finalRotation);
}

glm::vec3 Bone::interpolateScaling(float animationTime)
{
    if (1 == m_NumScalings)
        return m_Scales[0].scale;

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_Scales[p0Index].timestamp,
                                       m_Scales[p1Index].timestamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
    return finalScale;
}
