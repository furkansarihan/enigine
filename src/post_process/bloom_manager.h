#ifndef bloom_manager_hpp
#define bloom_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../shader/shader.h"

struct BloomMip
{
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

class BloomManager
{
public:
    BloomManager(Shader *downsampleShader, Shader *upsampleShader, unsigned int quad_vao);
    ~BloomManager();

    void updateResolution(unsigned int windowWidth, unsigned int windowHeight);
    void renderBloomTexture(unsigned int srcTexture);
    unsigned int bloomTexture();

    bool m_karisAverageOnDownsample;
    float m_threshold;
    float m_softThreshold;
    float m_filterRadius;

private:
    Shader *m_downsampleShader;
    Shader *m_upsampleShader;
    unsigned int m_quad_vao;

    unsigned int m_fbo;
    std::vector<BloomMip> m_mipChain;
    glm::ivec2 m_viewportSize;
    glm::vec2 m_viewportSizeFloat;

    void createTextures(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength);
    void deleteTextures();
    void renderDownsamples(unsigned int srcTexture);
    void renderUpsamples();
};

#endif /* bloom_manager_hpp */
