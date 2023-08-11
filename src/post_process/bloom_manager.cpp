#include "bloom_manager.h"

BloomManager::BloomManager(Shader *downsampleShader, Shader *upsampleShader, unsigned int quad_vao)
    : m_downsampleShader(downsampleShader),
      m_upsampleShader(upsampleShader),
      m_quad_vao(quad_vao),
      m_karisAverageOnDownsample(true),
      m_threshold(1.f),
      m_softThreshold(0.3f),
      m_filterRadius(0.005f)
{
    glGenFramebuffers(1, &m_fbo);
}

BloomManager::~BloomManager() {}

void BloomManager::createTextures(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glm::vec2 mipSize((float)windowWidth, (float)windowHeight);
    glm::ivec2 mipIntSize((int)windowWidth, (int)windowHeight);

    for (GLuint i = 0; i < mipChainLength; i++)
    {
        BloomMip mip;

        mipSize *= 0.5f;
        mipIntSize /= 2;
        mip.size = mipSize;
        mip.intSize = mipIntSize;

        if (mipIntSize.x < 8 || mipIntSize.y < 8)
            break;

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                     (int)mipSize.x, (int)mipSize.y,
                     0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // std::cout << "BloomManager: created bloom mip " << mipIntSize.x << 'x' << mipIntSize.y << std::endl;
        m_mipChain.emplace_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_mipChain[0].texture, 0);

    // setup attachments
    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // check completion status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("BloomManager: gbuffer FBO error, status: 0x%x\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomManager::deleteTextures()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    for (int i = 0; i < (int)m_mipChain.size(); i++)
    {
        glDeleteTextures(1, &m_mipChain[i].texture);
        m_mipChain[i].texture = 0;
    }
    m_mipChain.clear();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomManager::updateResolution(unsigned int windowWidth, unsigned int windowHeight)
{
    if (m_viewportSize.x == windowWidth && m_viewportSize.y == windowHeight)
        return;

    m_viewportSize = glm::ivec2(windowWidth, windowHeight);
    m_viewportSizeFloat = glm::vec2((float)windowWidth, (float)windowHeight);

    deleteTextures();

    const unsigned int num_bloom_mips = 6; // TODO: Play around with this value
    createTextures(windowWidth, windowHeight, num_bloom_mips);
}

void BloomManager::renderDownsamples(unsigned int srcTexture)
{
    const std::vector<BloomMip> &mipChain = m_mipChain;

    m_downsampleShader->use();
    m_downsampleShader->setVec2("srcResolution", m_viewportSizeFloat);
    if (m_karisAverageOnDownsample)
        m_downsampleShader->setInt("mipLevel", 0);
    m_downsampleShader->setFloat("threshold", m_threshold);
    m_downsampleShader->setFloat("softThreshold", m_softThreshold);

    // Bind srcTexture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // Progressively downsample through the mip chain
    for (int i = 0; i < (int)mipChain.size(); i++)
    {
        const BloomMip &mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(m_quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Set current mip resolution as srcResolution for next iteration
        m_downsampleShader->setVec2("srcResolution", mip.size);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // Disable Karis average for consequent downsamples
        if (i == 0)
            m_downsampleShader->setInt("mipLevel", 1);
    }

    glUseProgram(0);
}

void BloomManager::renderUpsamples()
{
    const std::vector<BloomMip> &mipChain = m_mipChain;

    m_upsampleShader->use();
    m_upsampleShader->setFloat("filterRadius", m_filterRadius);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = (int)mipChain.size() - 1; i > 0; i--)
    {
        const BloomMip &mip = mipChain[i];
        const BloomMip &nextMip = mipChain[i - 1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, nextMip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(m_quad_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    // Disable additive blending
    // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);

    glUseProgram(0);
}

void BloomManager::renderBloomTexture(unsigned int srcTexture)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    renderDownsamples(srcTexture);
    renderUpsamples();
}

GLuint BloomManager::bloomTexture()
{
    return m_mipChain[0].texture;
}
