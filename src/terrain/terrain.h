#ifndef terrain_hpp
#define terrain_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../shader/shader.h"
#include "../physics_world/physics_world.h"

class Terrain
{
public:
    // Contructors
    Terrain(PhysicsWorld *physicsWorld);
    ~Terrain();
    // Terrain Attributes
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
    btRigidBody *terrainBody;
    // Functions
    void draw(Shader terrainShader, glm::vec3 cameraPosition, glm::vec3 lightPosition, glm::mat4 viewProjection);

private:
    unsigned int vao_mxm, vao_mx3, vao_3xm, vao_2m1x2, vao_2x2m1, vao_0, vao_3x3, vao_2x2;
    unsigned textureID, ntextureID, ttextureID;
    int width, height;
    // TODO: naming
    float w, h;
    float *data;
    // Functions
    void createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    void createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo);
    int roundUp(int numToRound, int multiple);
};

#endif /* terrain_hpp */
