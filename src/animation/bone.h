#ifndef bone_hpp
#define bone_hpp

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>

#include "../utils/assimp_to_glm.h"

struct KeyVec3
{
    float timestamp;
    glm::vec3 value;
};

struct KeyQuat
{
    float timestamp;
    glm::quat value;
};

enum AnimationType
{
    Cycle,
    Pose
};

class Bone
{
public:
    std::vector<KeyVec3> m_positions;
    std::vector<KeyQuat> m_rotations;
    std::vector<KeyVec3> m_scales;
    int m_numPositions;
    int m_numRotations;
    int m_numScalings;

    glm::vec3 m_translation;
    glm::quat m_rotation;
    glm::vec3 m_scale;
    std::string m_name;
    int m_ID;

    AnimationType m_animType;
    float m_blendFactor = 1.0f;

    Bone(const std::string &name, int ID, const aiNodeAnim *channel, AnimationType m_animType);
    ~Bone();
    void update(float animationTime);
    void updatePose();
    void updateCycle(float animationTime);
    // TODO: single function without polimorphism?
    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::vec3 interpolatePosition(float animationTime);
    glm::quat interpolateRotation(float animationTime);
    glm::vec3 interpolateScaling(float animationTime);
};

#endif /* bone_hpp */
