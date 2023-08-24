#include "post_process.h"

PostProcess::PostProcess(int width, int heigth)
    : m_width(width),
      m_height(heigth),
      m_exposure(1.0f),
      m_contrastBright(0.25f),
      m_contrastDark(0.2f),
      m_bloomIntensity(0.06f)
{
    glGenFramebuffers(1, &m_framebufferObject);

    this->createTexture();
}

PostProcess::~PostProcess()
{
    // TODO: destruction
}

void PostProcess::createTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "PostProcess: Framebuffer creation error!\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_texture = texture;
}

void PostProcess::updateResolution(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);

    // unbind texture, renderbuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // delete texture, renderbuffer
    // TODO: verify, handle error?
    glDeleteTextures(1, &m_texture);
    glDeleteRenderbuffers(1, &m_depthRenderbuffer);

    m_width = width;
    m_height = height;

    createTexture();
}
