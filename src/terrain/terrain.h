#ifndef terrain_hpp
#define terrain_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../shader/shader.h"
#include "../physics_world/physics_world.h"
#include "../camera/camera.h"

class Terrain
{
public:
    Terrain(PhysicsWorld *physicsWorld, Camera *camera);
    ~Terrain();

    Camera *m_camera;
    int resolution;
    bool wireframe;
    glm::vec3 terrainCenter;
    int level;
    float scaleFactor;
    float fogMaxDist;
    float fogMinDist;
    glm::vec4 fogColor;
    glm::vec2 uvOffset;
    glm::vec2 alphaOffset;
    float oneOverWidth;
    glm::vec3 shadowBias;
    bool showCascade;
    btRigidBody *terrainBody;

    void drawDepth(Shader terrainShadow, glm::mat4 view, glm::mat4 projection);
    void drawColor(Shader terrainShader, glm::vec3 lightPosition, glm::vec3 lightColor, float lightPower,
                   glm::mat4 view, glm::mat4 projection, GLuint shadowmapId, glm::vec3 camPos, glm::vec3 camView, glm::vec4 frustumDistances);

private:
    unsigned int vao_mxm, vao_3xm, vao_2m1x2, vao_2x2m1, vao_0, vao_tf, vao_3x3;
    unsigned textureID, ntextureID, ttextureID;
    int width, height;
    // TODO: naming
    float w, h;
    float *data;

    void createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createTriangleFanMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    int roundUp(int numToRound, int multiple);
    void draw(Shader shader);
    void drawBlock(Shader shader, unsigned int vao, int scale, glm::vec2 size, glm::vec2 pos, int indiceCount);

    // frustum culling
    glm::vec4 m_planes[4];
    void calculatePlanes(glm::mat4 projMatrix, glm::mat4 viewMatrix);
    bool inFrontOf(glm::vec4 plane, glm::vec3 corners[8]);
};

#endif /* terrain_hpp */
