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
    float fov;
    float ratio;
    glm::vec3 points[8];
    glm::vec3 lightAABB[8];
};

class ShadowManager
{
public:
    ShadowManager(std::vector<unsigned int> shaderIds);
    ~ShadowManager();

    Camera *m_camera;
    std::vector<frustum> m_frustums;
    int m_splitCount = 3;
    float m_splitWeight = 0.75f;
    float m_near = 0.1f;
    float m_far = 200.0f;

    std::vector<unsigned int> m_shaderIds;

    glm::vec3 m_lightPos = glm::vec3(0.35f, 0.35f, 0.4f);
    glm::vec3 m_lightLookAt = glm::vec3(0, 0, 0);

    std::vector<glm::mat4> m_depthPMatrices;

    glm::mat4 getDepthViewMatrix();
    glm::vec4 getFrustumDistances();
    void setup(float screenWidth, float screenHeight);

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
    glm::mat4 applyCropMatrix(frustum &f, glm::mat4 lightView);
    void setupBiasMatrices(glm::mat4 depthViewMatrix);
};

#endif /* shadow_manager_hpp */
