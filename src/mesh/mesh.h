#ifndef mesh_hpp
#define mesh_hpp

#include <string>

#include "../material/material.h"
#include "../shader/shader.h"

#define MAX_BONE_PER_VERTEX 4
namespace enigine
{
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
} // namespace enigine
using namespace enigine;

class Mesh
{
public:
    // Constructors
    Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, glm::vec3 aabbMin, glm::vec3 aabbMax, Material *material);
    Mesh()
    {
    }
    ~Mesh();
    // Attributes
    unsigned int VAO, VBO, EBO;
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    Material *material;
    glm::mat4 offset = glm::mat4(1.0f);
    // Functions
    void draw(Shader shader);
    void drawInstanced(Shader shader, int instanceCount);

    void setupMesh();
    void bindTextures(Shader shader);
    void unbindTextures(Shader shader);
    void bindProperties(Shader shader);
    void unbindProperties(Shader shader);
};

#endif /* mesh_hpp */
