#ifndef animation_hpp
#define animation_hpp

#include <vector>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include "bone.h"
#include "../model/model.h"
#include "../utils/assimp_to_glm.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    float m_Duration;
    int m_TicksPerSecond;
    std::unordered_map<std::string, Bone*> m_bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    Animation(const std::string &animationPath, Model *model);
    ~Animation();
    Bone *getBone(const std::string &name);
    void readBones(const aiAnimation *animation, Model &model);
    void readHierarchy(AssimpNodeData &dest, const aiNode *src);
};

#endif /* animation_hpp */
