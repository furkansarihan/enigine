#ifndef mesh_hpp
#define mesh_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <assimp/scene.h>

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

struct MaterialProperty
{
    std::string name;
    int type;
    std::string value;

    MaterialProperty(const std::string &name, int type, const std::string &value)
        : name(name), type(type), value(value) {}
};

struct Material
{
    std::string name;
    std::vector<Texture> textures;
    std::vector<MaterialProperty> properties;

    Material(const std::string &name, std::vector<Texture> &textures, std::vector<MaterialProperty> &properties)
        : name(name), textures(textures), properties(properties) {}
};

class Mesh
{
public:
    // Constructors
    Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, glm::vec3 aabbMin, glm::vec3 aabbMax, Material material);
    ~Mesh();
    // Attributes
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    Material material;
    glm::mat4 offset = glm::mat4(1.0f);
    bool opaque = true;
    unsigned int VAO;
    // Functions
    void draw(Shader shader);
    void drawInstanced(Shader shader, int instanceCount);

private:
    unsigned int VBO, EBO;
    void setupMesh();
    void updateTransmission();
    void bindTextures(Shader shader);
    void unbindTextures(Shader shader);
    void bindProperties(Shader shader);
    void unbindProperties(Shader shader);
};

#endif /* mesh_hpp */
