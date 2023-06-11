#ifndef mesh_hpp
#define mesh_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include "../shader/shader.h"

#define MAX_BONE_PER_VERTEX 4

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int boneIDs[MAX_BONE_PER_VERTEX];
    float weights[MAX_BONE_PER_VERTEX];
};

struct Texture
{
    unsigned int id;
    std::string type;
};

class Mesh
{
public:
    // Constructors
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    ~Mesh();
    // Attributes
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;
    // Functions
    void draw(Shader shader);
    void drawInstanced(Shader shader, int instanceCount);

private:
    unsigned int VBO, EBO;
    void setupMesh();
    void bindTextures(Shader shader);
};

#endif /* mesh_hpp */
