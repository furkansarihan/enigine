#ifndef ssao_hpp
#define ssao_hpp

#include <iostream>
#include <vector>
#include <random>

#include <GL/glew.h>
#include <glm/glm.hpp>

class SSAO
{
public:
    SSAO(int width, int height);
    ~SSAO();

    unsigned int ssaoFBO, ssaoBlurFBO;
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

    unsigned int noiseTexture;
    int noiseSize = 4;
    std::vector<glm::vec3> ssaoKernel;

    // TODO: adaptive params 
    // outdoor - indoor
    // close - far, from surface
    int kernelSize = 64;
    float radius = 0.5f;
    float bias = 0.050f;
    float strength = 3.0f;

    int m_width;
    int m_height;

    void updateResolution(int width, int height);

private:
    void setup();
    void createTextures();
};

#endif /* ssao_hpp */
