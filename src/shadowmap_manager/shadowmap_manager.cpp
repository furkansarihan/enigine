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

    glViewport(0, 0, m_shadowmapSize, m_shadowmapSize);
}

void ShadowmapManager::bindTextureArray(int index)
{
    // Bind texture array
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

    // Bind layer to framebuffer
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureArray, 0, index);

    // Clear value for depth buffer
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // No color output for shadow map
    glDrawBuffer(GL_NONE);
}

void ShadowmapManager::updateSize(int size)
{
    if (m_shadowmapSize == size)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glDeleteTextures(1, &m_textureArray);

    m_shadowmapSize = size;

    createTextureArray();
}
