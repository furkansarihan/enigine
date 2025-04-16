#ifndef shadowmap_manager_hpp
#define shadowmap_manager_hpp

#include <sstream>
#include <GL/glew.h>

class ShadowmapManager
{
public:
    ShadowmapManager(int cascadeCount, int shadowmapSize);
    ~ShadowmapManager();

    GLuint m_textureArray;

    void bindFramebuffer();
    void bindTextureArray(int index);
    void updateSize(int size);

    int getSize()
    {
        return m_shadowmapSize;
    }

private:
    GLuint m_framebufferObject;
    int m_cascadeCount;
    int m_shadowmapSize;

    void createTextureArray();
};

#endif /* shadowmap_manager_hpp */
