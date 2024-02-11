#ifndef pbr_manager_hpp
#define pbr_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../shader/shader.h"
#include "../model/model.h"

class PbrManager
{
public:
    PbrManager();
    ~PbrManager();

    int m_cubemapFaceSize;
    int m_irradianceSize;
    int m_prefilterSize;
    int m_brdfLutSize;

    Texture m_environmentTexture;

    unsigned int captureFBO, captureRBO;

    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUTTexture;

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[6] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    };

    void setupCubemap(Model *cube, Shader &hdrToCubemapShader);
    void setupIrradianceMap(Model *cube, Shader &irradianceShader);
    void setupPrefilterMap(Model *cube, Shader &prefilterShader);
    void setupBrdfLUTTexture(unsigned int quadVAO, Shader &brdfShader);
};

#endif /* pbr_manager_hpp */
