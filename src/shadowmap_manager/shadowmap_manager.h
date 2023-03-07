#ifndef shadowmap_manager_hpp
#define shadowmap_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <GL/glew.h>

class ShadowmapManager
{
public:
    ShadowmapManager(int cascadeCount, int shadowmapSize);
    ~ShadowmapManager();

    GLuint m_textureArray;

    void bindFramebuffer();
    void bindTextureArray(int index);

private:
    GLuint m_framebufferObject;
    int m_cascadeCount;
    int m_shadowmapSize;

    void createTextureArray();
};

#endif /* shadowmap_manager_hpp */
