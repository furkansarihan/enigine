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
    PostProcess(int width, int height);
    ~PostProcess();

    GLuint m_framebufferObject;
    GLuint m_texture;
    GLuint m_depthRenderbuffer;
    int m_width;
    int m_height;

    float m_exposure;
    float m_contrastBright;
    float m_contrastDark;
    float m_bloomIntensity;

    void updateResolution(int width, int height);

private:
    void createTexture();
};

#endif /* post_process_hpp */
