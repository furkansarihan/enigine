#ifndef model_hpp
#define model_hpp

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../mesh/mesh.h"
#include "../shader/shader.h"
#include "../utils/assimp_to_glm.h"

class ResourceManager;
#include "../resource_manager/resource_manager.h"

struct BoneInfo
{
    // id is index in finalBoneMatrices
    int id;

    // offset matrix transforms vertex from model space to bone space
    glm::mat4 offset;
};

class Model
{
public:
    Model(ResourceManager *resourceManager, std::string const &path);
    Model(Model *model, int meshIndex);
    Model();
    ~Model();

    std::vector<Mesh *> meshes;
    std::vector<Mesh *> opaqueMeshes;
    std::vector<Mesh *> transmissionMeshes;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    std::string m_path;
    std::string m_directory;
    bool gammaCorrection;

    std::map<std::string, BoneInfo> m_boneInfoMap;
    int m_boneCounter = 0;

    ResourceManager *m_resourceManager;
    Assimp::Importer *m_importer;
    const aiScene *m_scene;

    void draw(Shader shader, bool drawOpaque = true);
    void drawInstanced(Shader shader, int instanceCount);
    void updateMeshTypes();

private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, glm::mat4 parentTransform);
    Mesh *processMesh(aiMesh *mesh);
    Material *loadMaterial(aiMaterial *material);
    std::vector<Texture *> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    void setVertexBoneDataToDefault(Vertex &vertex);
    void setVertexBoneData(Vertex &vertex, int boneID, float weight);
    void extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh);
};

#endif /* model_hpp */
