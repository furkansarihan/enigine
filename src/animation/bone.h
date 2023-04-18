#ifndef bone_hpp
#define bone_hpp

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>

#include "../utils/assimp_to_glm.h"

struct KeyPosition
{
    float timestamp;
    glm::vec3 position;
};

struct KeyRotation
{
    float timestamp;
    glm::quat orientation;
};

struct KeyScale
{
    float timestamp;
    glm::vec3 scale;
};

class Bone
{
public:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    /*reads keyframes from aiNodeAnim*/
    Bone(const std::string &name, int ID, const aiNodeAnim *channel);
    ~Bone();
    void update(float animationTime);
    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 interpolatePosition(float animationTime);
    glm::mat4 interpolateRotation(float animationTime);
    glm::mat4 interpolateScaling(float animationTime);
};

#endif /* bone_hpp */
