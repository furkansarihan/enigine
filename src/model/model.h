#ifndef model_hpp
#define model_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../shader/shader.h"
#include "../mesh/mesh.h"
#include "../utils/assimp_to_glm.h"

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
    Model(std::string const &path);
    ~Model();
    void draw(Shader shader);
    void drawInstanced(Shader shader, int instanceCount);
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    std::map<std::string, BoneInfo> m_boneInfoMap;
    int m_boneCounter = 0;

private:
    const aiScene *m_scene;

    void loadModel(std::string const &path);
    void processNode(aiNode *node);
    Mesh processMesh(aiMesh *mesh);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    void setVertexBoneDataToDefault(Vertex &vertex);
    void setVertexBoneData(Vertex &vertex, int boneID, float weight);
    void extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh);
};

#endif /* model_hpp */
