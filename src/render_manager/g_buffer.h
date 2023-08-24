#ifndef g_buffer_hpp
#define g_buffer_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

class GBuffer
{
public:
    GBuffer(int width, int height);
    ~GBuffer();

    unsigned int m_fbo;
    unsigned int m_rboDepth;
    unsigned int m_gPosition, m_gNormalShadow, m_gAlbedo, m_gAoRoughMetal;
    unsigned int m_gViewPosition, m_gViewNormal;

    int m_width;
    int m_height;

    void updateResolution(int width, int height);

private:
    void createTextures();
};

#endif /* g_buffer_hpp */
