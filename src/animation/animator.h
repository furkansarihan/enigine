#ifndef animator_hpp
#define animator_hpp

#include <vector>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>

#include "animation.h"

#define MAX_BONES 150

class Animator
{
public:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation *m_CurrentAnimation;
    float m_CurrentTime;

    Animator(Animation *animation);
    ~Animator();
    void update(float deltaTime);
    void play(Animation *pAnimation);
    void calculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform);
};

#endif /* animator_hpp */
