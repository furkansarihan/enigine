#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include "bone.h"

Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel)
    : m_name(name),
      m_ID(ID),
      m_translation(1.0f),
      m_rotation(glm::vec3(1.0f)),
      m_scale(1.0f)
{
    m_numPositions = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < m_numPositions; ++positionIndex)
    {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        KeyVec3 key;
        key.timestamp = channel->mPositionKeys[positionIndex].mTime;
        key.value = AssimpToGLM::getGLMVec3(aiPosition);
        m_positions.push_back(key);
    }

    m_numRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_numRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        KeyQuat key;
        key.timestamp = channel->mRotationKeys[rotationIndex].mTime;
        key.value = AssimpToGLM::getGLMQuat(aiOrientation);
        m_rotations.push_back(key);
    }

    m_numScalings = channel->mNumScalingKeys;
    for (int scaleIndex = 0; scaleIndex < m_numScalings; ++scaleIndex)
    {
        aiVector3D scale = channel->mScalingKeys[scaleIndex].mValue;
        KeyVec3 key;
        key.timestamp = channel->mScalingKeys[scaleIndex].mTime;
        key.value = AssimpToGLM::getGLMVec3(scale);
        m_scales.push_back(key);
    }

    if (m_numPositions == 1 && m_numRotations == 1 && m_numScalings == 1)
    {
        m_animType = AnimationType::Pose;

        m_translation = m_positions[0].value;
        m_rotation = m_rotations[0].value;
        m_scale = m_scales[0].value;
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
    for (int index = 0; index < m_numPositions - 1; ++index)
    {
        if (animationTime < m_positions[index + 1].timestamp)
            return index;
    }
    assert(0);
}

int Bone::getRotationIndex(float animationTime)
{
    for (int index = 0; index < m_numRotations - 1; ++index)
    {
        if (animationTime < m_rotations[index + 1].timestamp)
            return index;
    }
    assert(0);
}

int Bone::getScaleIndex(float animationTime)
{
    for (int index = 0; index < m_numScalings - 1; ++index)
    {
        if (animationTime < m_scales[index + 1].timestamp)
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
    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_positions[p0Index].timestamp, m_positions[p1Index].timestamp, animationTime);
    return glm::mix(m_positions[p0Index].value, m_positions[p1Index].value, scaleFactor);
}

glm::quat Bone::interpolateRotation(float animationTime)
{
    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_rotations[p0Index].timestamp, m_rotations[p1Index].timestamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_rotations[p0Index].value, m_rotations[p1Index].value, scaleFactor);
    // TODO: ?
    return glm::normalize(finalRotation);
}

glm::vec3 Bone::interpolateScaling(float animationTime)
{
    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = getScaleFactor(m_scales[p0Index].timestamp, m_scales[p1Index].timestamp, animationTime);
    return glm::mix(m_scales[p0Index].value, m_scales[p1Index].value, scaleFactor);
}
