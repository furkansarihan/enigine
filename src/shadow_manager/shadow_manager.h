#ifndef shadow_manager_hpp
#define shadow_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "../camera/camera.h"

struct frustum
{
    float near;
    float far;
    float farExtend;
    float fov;
    float ratio;
    glm::vec3 points[8];
    glm::vec3 lightAABB[8];
};

struct aabb
{
    glm::vec3 min;
    glm::vec3 max;

    aabb(){};
    aabb(glm::vec3 min, glm::vec3 max) : min(min), max(max){};
};

struct SceneObject
{
    glm::vec3 center;
    float radius;
    std::vector<int> frustumIndexes;
};

class ShadowManager
{
public:
    ShadowManager(Camera *camera, std::vector<unsigned int> shaderIds);
    ~ShadowManager();

    Camera *m_camera;
    std::vector<frustum> m_frustums;
    std::vector<float> m_frustumDistances;
    std::vector<SceneObject> m_sceneObjects;
    aabb m_aabb;
    int m_splitCount = 3;
    float m_splitWeight = 0.75f;
    float m_near = 0.1f;
    float m_far = 200.0f;

    std::vector<unsigned int> m_shaderIds;

    glm::vec3 m_lightPos = glm::vec3(0.333f, 0.9f, 0.28f);
    glm::vec3 m_lightLookAt = glm::vec3(0, 0, 0);

    std::vector<glm::mat4> m_depthPMatrices;

    glm::mat4 getDepthViewMatrix();
    void setupFrustum(float screenWidth, float screenHeight, glm::mat4 projection);
    void setupLightAabb(const std::vector<aabb> &objectAabbs);

private:
    unsigned int m_ubo;
    glm::mat4 m_biasMatrix = glm::mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0);

    void setupUBO();
    void updateSplitDist(float nd, float fd);
    void updateFrustumPoints(frustum &f, glm::vec3 &center, glm::vec3 &view_dir);
    glm::mat4 applyCropMatrix(int frustumIndex, glm::mat4 lightView);
    void setupBiasMatrices(glm::mat4 depthViewMatrix);
    void updateFrustumAabb();
};

#endif /* shadow_manager_hpp */
