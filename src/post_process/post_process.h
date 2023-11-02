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

    float m_contrastBright;
    float m_contrastDark;
    float m_bloomIntensity;

    // tone mapping
    float m_A; // Shoulder Strength
    float m_B; // Linear Strength
    float m_C; // Linear Angle
    float m_D; // Toe Strength
    float m_E; // Toe Numerator
    float m_F; // Toe Denominator
    float m_W; // Linear White
    float m_exposure;

    void updateResolution(int width, int height);

private:
    void createTexture();
};

#endif /* post_process_hpp */
