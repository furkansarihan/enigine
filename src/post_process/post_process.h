#ifndef post_process_hpp
#define post_process_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

class PostProcess
{
public:
    PostProcess(float width, float height);
    ~PostProcess();

    GLuint m_framebufferObject;
    GLuint m_texture;
    GLuint m_depthRenderbuffer;
    float m_width;
    float m_height;

    float m_exposure = 1.0f;
    float m_contrastBright = 1.0f;
    float m_contrastDark = 0.25f;

    void updateFramebuffer(float width, float height);

private:
    void createTexture();
};

#endif /* post_process_hpp */
