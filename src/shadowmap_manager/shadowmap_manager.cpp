#include "shadowmap_manager.h"

ShadowmapManager::ShadowmapManager(int cascadeCount, int shadowmapSize)
{
    m_cascadeCount = cascadeCount;
    m_shadowmapSize = shadowmapSize;

    // Single framebuffer for each cascade
    glGenFramebuffers(1, &m_framebufferObject);

    this->createTextureArray();
}

ShadowmapManager::~ShadowmapManager()
{
    // TODO: destruction
}

void ShadowmapManager::createTextureArray()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

    glGenTextures(1, &m_textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowmapSize, m_shadowmapSize, m_cascadeCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // out of shadowmap bound - white - far away
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    // for sample2DShadow
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    for (int i = 0; i < m_cascadeCount; i++)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureArray, 0, i);
    }

    glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        // TODO: exception
        fprintf(stderr, "ShadowmapManager: Framebuffer creation error!\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowmapManager::bindFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
    GLfloat clearDepth[] = {1.0f};

    glClearBufferfv(GL_DEPTH, 0, clearDepth);
    glViewport(0, 0, m_shadowmapSize, m_shadowmapSize);
}

void ShadowmapManager::bindTextureArray(int index)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

    // TODO: better way?
    // Create an array of floating-point depth values
    float *depthData = new float[m_shadowmapSize * m_shadowmapSize];
    std::fill_n(depthData, m_shadowmapSize * m_shadowmapSize, 1.0f);

    // Set the texture's data to the depth array
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, m_shadowmapSize, m_shadowmapSize, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depthData);

    // Delete the depth data array
    delete[] depthData;

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureArray, 0, index);

    glDrawBuffer(GL_NONE);
}
