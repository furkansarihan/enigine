#ifndef mesh_hpp
#define mesh_hpp

#include <string>

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
    int width, height, nrComponents;
    std::string type; // TODO: change
    glm::vec2 uvScale = glm::vec2(1.f);
};

enum MaterialBlendMode
{
    opaque,
    alphaBlend,
};

// TODO: same instance inside model
class Material
{
public:
    std::string name;
    std::vector<Texture> textures;

    MaterialBlendMode blendMode;

    glm::vec2 uvScale;
    glm::vec4 albedo;
    float roughness;
    float metallic;
    float transmission;
    float opacity;
    float ior;
    // emissive
    glm::vec4 emissiveColor;
    float emissiveStrength;
    // volume
    float thickness;

    // parallax occlusion mapping
    float parallaxMapMidLevel;
    float parallaxMapScale;
    float parallaxMapSampleCount;
    float parallaxMapScaleMode;

    Material(const std::string &name, std::vector<Texture> &textures);
};

class Mesh
{
public:
    // Constructors
    Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, glm::vec3 aabbMin, glm::vec3 aabbMax, Material *material);
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

private:
    void setupMesh();
    void bindTextures(Shader shader);
    void unbindTextures(Shader shader);
    void bindProperties(Shader shader);
    void unbindProperties(Shader shader);
};

#endif /* mesh_hpp */
