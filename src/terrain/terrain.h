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

#include "../shader_manager/shader_manager.h"
#include "../physics_world/physics_world.h"
#include "../camera/camera.h"
#include "../model/model.h"
#include "../pbr_manager/pbr_manager.h"
#include "../render_manager/render_manager.h"

struct HeightCell
{
    float min;
    float max;
    HeightCell(float min, float max) : min(min), max(max){};
};

// TODO: naming variables
class Terrain : public Renderable
{
public:
    Terrain(RenderManager *renderManager, ResourceManager *resourceManager, ShaderManager *shaderManager,
            PhysicsWorld *physicsWorld, const std::string &heightmapFilename, float minHeight, float maxHeight,
            float scaleHoriz);
    ~Terrain();

    RenderManager *m_renderManager;
    ResourceManager *m_resourceManager;
    ShaderManager *m_shaderManager;
    PhysicsWorld *m_physicsWorld;
    std::string m_heightmapFilename;
    int heightmapWidth, heightmapHeight;
    float width, height;
    float w, h;

    int resolution = 128;
    float m_minHeight, m_maxHeight, m_scaleHoriz;

    Shader terrainPBRShader;
    Shader terrainDepthShader;

    // TODO: ?
    Texture m_diffuseArray;
    Texture m_normalArray;
    Texture m_aoArray;
    Texture m_roughArray;
    Texture m_heightArray;

    int level = 9;
    glm::vec2 m_worldOrigin = glm::vec2(0.0f, 0.0f);
    bool showCascade = false;
    bool wireframe = false;
    bool m_debugCulling = false;
    btRigidBody *terrainBody;

    Model *m_grass;
    Model *m_stone;
    Shader m_grassShader;
    Shader m_stoneShader;

    int m_grassTileSize = 12;
    float m_grassDensity = 2;
    glm::vec3 m_grassColorFactor = glm::vec3(1.35f, 1.11f, 1.17f);
    int m_stoneTileSize = 56;
    float m_stoneDensity = 0.02;
    float m_windIntensity = 6.0;
    // TODO: obstacle map - grass
    glm::vec3 m_playerPos;

    float ambientMult = 0.5f;
    float diffuseMult = 0.7f;
    float specularMult = 0.15f;

    // frustum culling
    std::vector<std::vector<HeightCell>> m_heightMatrix;
    std::vector<std::vector<bool>> m_heightMatrixCulled;
    float m_heightCellSize = 64.f;
    int m_horizontalCellCount;
    int m_verticalCellCount;
    float m_cellScaleMult = 1.0f;
    bool m_drawHeightCells = false;

    void renderDepth();
    void renderColor();

    void drawDepth(Shader terrainShadow, glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos);
    void drawColor(PbrManager *pbrManager, Shader terrainShader, glm::vec3 lightPosition, glm::vec3 lightColor, float lightPower,
                   glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos,
                   glm::mat4 cullView, glm::mat4 cullProjection, glm::vec3 cullViewPos,
                   GLuint shadowmapId, glm::vec3 camPos, glm::vec3 camView, glm::vec4 frustumDistances, glm::vec3 shadowBias,
                   bool ortho);
    void drawInstance(glm::vec3 grassColorFactor, glm::vec3 playerPos, Shader instanceShader, Model *model, int tileSize, float density, glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos);
    void updateHorizontalScale();

private:
    unsigned int vao_mxm, vao_3xm, vao_2m1x2, vao_2x2m1, vao_0, vao_tf, vao_3x3;
    unsigned int textureID;
    float *data;

    void init();
    int roundUp(int numToRound, int multiple);
    float roundUpf(float numToRound, float multiple);
    void createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createTriangleFanMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void draw(Shader shader, glm::vec3 viewPos, bool ortho);
    void drawBlock(Shader shader, unsigned int vao, int scale, glm::vec2 size, glm::vec2 pos, int indiceCount, glm::vec3 viewPos, bool ortho);

    // frustum culling
    glm::vec4 m_planes[5];
    void updateHeightMatrix();
    HeightCell getHeightRange(glm::vec2 topLeft, glm::vec2 bottomRight);
    void markHeightRange(glm::vec2 topLeft, glm::vec2 bottomRight, bool culled);
    glm::vec2 getCellIndex(glm::vec2 point);
    void calculatePlanes(glm::mat4 projMatrix, glm::mat4 viewMatrix);
    bool inFrustum(glm::vec2 topLeft, glm::vec2 topRight, glm::vec2 bottomLeft, glm::vec2 bottomRight, glm::vec3 viewPos, bool ortho);
    bool inFrontOf(glm::vec4 plane, glm::vec3 corners[8], glm::vec3 viewPos, bool ortho);
};

#endif /* terrain_hpp */
