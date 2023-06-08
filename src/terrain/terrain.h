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
#include "../model/model.h"
#include "../pbr_manager/pbr_manager.h"

// TODO: naming variables
class Terrain
{
public:
    Terrain(PbrManager *pbrManager, PhysicsWorld *physicsWorld, const std::string &filename, float minHeight, float maxHeight, float scaleHoriz, bool PBR);
    ~Terrain();

    PbrManager *m_pbrManager;
    int heightmapWidth, heightmapHeight;
    float width, height;
    float w, h;

    int resolution = 128;
    float m_minHeight, m_maxHeight, m_scaleHoriz;
    bool m_PBR;
    glm::vec3 terrainCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    int level = 9;
    float fogMaxDist = 10000.0f;
    float fogMinDist = 4500.0f;
    glm::vec4 fogColor = glm::vec4(1.f, 1.f, 1.f, 0.75f);
    glm::vec2 uvOffset = glm::vec2(0.0f, 0.0f);
    glm::vec3 shadowBias = glm::vec3(0.020, 0.023, 0.005);
    bool showCascade = false;
    bool wireframe = false;
    btRigidBody *terrainBody;

    int m_grassTileSize = 12;
    float m_grassDensity = 2;
    int m_stoneTileSize = 56;
    float m_stoneDensity = 0.02;
    float m_windIntensity = 6.0;

    float ambientMult = 0.5f;
    float diffuseMult = 0.7f;
    float specularMult = 0.15f;

    void drawDepth(Shader terrainShadow, glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos);
    void drawColor(Shader terrainShader, glm::vec3 lightPosition, glm::vec3 lightColor, float lightPower,
                   glm::mat4 view, glm::mat4 projection,
                   GLuint shadowmapId, glm::vec3 camPos, glm::vec3 camView, glm::vec4 frustumDistances,
                   glm::vec3 viewPos, bool ortho);
    void drawInstance(Shader instanceShader, Model *model, int tileSize, float density, glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos);
    void updateHorizontalScale();

private:
    unsigned int vao_mxm, vao_3xm, vao_2m1x2, vao_2x2m1, vao_0, vao_tf, vao_3x3;
    unsigned int textureID;
    unsigned int normalTextureArrayId;
    unsigned int PBRTextureArrayIds[5];
    float *data;

    void initTextureArray(unsigned int &textureArrayId, std::string *texturePaths);
    void setupAnisotropicFiltering();
    int roundUp(int numToRound, int multiple);
    float roundUpf(float numToRound, float multiple);
    void createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createTriangleFanMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void draw(Shader shader, glm::vec3 viewPos, bool ortho);
    void drawBlock(Shader shader, unsigned int vao, int scale, glm::vec2 size, glm::vec2 pos, int indiceCount, glm::vec3 viewPos, bool ortho);

    // frustum culling
    glm::vec4 m_planes[4];
    void calculatePlanes(glm::mat4 projMatrix, glm::mat4 viewMatrix);
    bool inFrustum(glm::vec2 topLeft, glm::vec2 topRight, glm::vec2 bottomLeft, glm::vec2 bottomRight, glm::vec3 viewPos, bool ortho);
    bool inFrontOf(glm::vec4 plane, glm::vec3 corners[8], glm::vec3 viewPos, bool ortho);
};

#endif /* terrain_hpp */
