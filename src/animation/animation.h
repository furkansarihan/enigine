#ifndef animation_hpp
#define animation_hpp

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>
#include "btBulletDynamicsCommon.h"

#include "bone.h"
#include "../model/model.h"
#include "../utils/assimp_to_glm.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    std::vector<AssimpNodeData *> children;
};

class Animation
{
public:
    std::string m_name;
    float m_duration;
    int m_ticksPerSecond;

    AssimpNodeData *m_rootNode;
    std::map<std::string, AssimpNodeData *> m_assimpNodes;

    std::unordered_map<std::string, Bone *> m_bones;
    std::map<std::string, BoneInfo> m_boneInfoMap;
    std::unordered_map<std::string, float> m_blendMask;

    Animation(const std::string &animationName, Model *model);
    ~Animation();
    Bone *getBone(const std::string &name);
    void readBones(const aiAnimation *animation, Model &model);
    void readHierarchy(AssimpNodeData *dest, const aiNode *src);
    void setBlendMask(std::unordered_map<std::string, float> blendMask, float defaultValue);
};

#endif /* animation_hpp */
