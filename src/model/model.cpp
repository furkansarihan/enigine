#include "model.h"

Model::Model(ResourceManager *resourceManager, std::string const &path, bool useOffset)
    : m_resourceManager(resourceManager),
      m_path(path),
      m_useOffset(useOffset)
{
    m_importer = new Assimp::Importer();

    loadModel(path);
}

Model::~Model()
{
    delete m_importer;

    for (int i = 0; i < meshes.size(); i++)
        delete meshes[i];
}

void Model::draw(Shader shader, bool drawOpaque)
{
    if (drawOpaque)
    {
        for (unsigned int i = 0; i < opaqueMeshes.size(); i++)
        {
            shader.setMat4("u_meshOffset", opaqueMeshes[i]->offset);
            opaqueMeshes[i]->draw(shader);
        }
    }
    else
    {
        for (unsigned int i = 0; i < transmissionMeshes.size(); i++)
        {
            shader.setMat4("u_meshOffset", transmissionMeshes[i]->offset);
            transmissionMeshes[i]->draw(shader);
        }
    }
}

void Model::drawInstanced(Shader shader, int instanceCount)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        shader.setMat4("u_meshOffset", meshes[i]->offset);
        meshes[i]->drawInstanced(shader, instanceCount);
    }
}

void Model::loadModel(std::string const &path)
{
    // read file via ASSIMP
    m_scene = m_importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << m_importer->GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    for (unsigned int i = 0; i < m_scene->mNumTextures; i++)
    {
        aiTexture *texture = m_scene->mTextures[i];
        std::cout << "path: " << texture->mFilename.C_Str() << ", format: " << texture->achFormatHint << std::endl;
    }

    // process ASSIMP's root node recursively
    processNode(m_scene->mRootNode);
}

void Model::processNode(aiNode *node)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *assimpMesh = m_scene->mMeshes[node->mMeshes[i]];

        Mesh *mesh = processMesh(assimpMesh);
        if (m_useOffset)
            mesh->offset = AssimpToGLM::getGLMMat4(node->mTransformation);

        meshes.push_back(mesh);
        if (mesh->opaque)
            opaqueMeshes.push_back(mesh);
        else
            transmissionMeshes.push_back(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i]);
}

Mesh *Model::processMesh(aiMesh *mesh)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        setVertexBoneDataToDefault(vertex);
        vertex.position = AssimpToGLM::getGLMVec3(mesh->mVertices[i]);
        vertex.normal = AssimpToGLM::getGLMVec3(mesh->mNormals[i]);
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        // tangent
        if (mesh->mTangents) // does the mesh contain Tangents
        {
            glm::vec3 vector;
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        }
        // bitangent
        if (mesh->mBitangents) // does the mesh contain Bitangents
        {
            glm::vec3 vector;
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        }
        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // TODO: merge detection for metal-rough-ao

    // material properties
    std::vector<MaterialProperty> properties;
    for (unsigned int i = 0; i < material->mNumProperties; i++)
    {
        const aiMaterialProperty *property = material->mProperties[i];
        std::string propertyName = property->mKey.C_Str();
        std::string type = propertyName.substr(1, 3);
        // TODO: check other properties
        if (type != "mat")
            continue;

        // TODO: depends on assimp - don't
        propertyName = propertyName.substr(5);
        std::replace(propertyName.begin(), propertyName.end(), '.', '_');
        std::string propertyValue;

        if (property->mType == aiPTI_Float)
        {
            float value = *reinterpret_cast<float *>(property->mData);
            propertyValue = std::to_string(value);
        }
        else if (property->mType == aiPTI_Integer)
        {
            int value = *reinterpret_cast<int *>(property->mData);
            propertyValue = std::to_string(value);
        }
        else
        {
            propertyValue = "unknown-type";
        }

        properties.push_back(MaterialProperty(propertyName, property->mType, propertyValue));
    }

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    // 5. roughness maps
    std::vector<Texture> rougnessMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, "texture_rough");
    textures.insert(textures.end(), rougnessMaps.begin(), rougnessMaps.end());
    // 6. ambient occlusion maps
    std::vector<Texture> aoMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ao");
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    // 7. metalness maps
    std::vector<Texture> metalMaps = loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metal");
    textures.insert(textures.end(), metalMaps.begin(), metalMaps.end());
    // 8. unknown maps
    std::vector<Texture> unknownMaps = loadMaterialTextures(material, aiTextureType_UNKNOWN, "texture_unknown");
    textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());

    // animation
    extractBoneWeightForVertices(vertices, mesh);

    return new Mesh(mesh->mName.C_Str(), vertices, indices, Material(material->GetName().C_Str(), textures, properties));
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string path = std::string(str.C_Str());
        Texture texture = m_resourceManager->getTexture(*this, path, typeName);
        textures.push_back(texture);
    }
    return textures;
}

void Model::setVertexBoneDataToDefault(Vertex &vertex)
{
    for (int i = 0; i < MAX_BONE_PER_VERTEX; i++)
    {
        vertex.boneIDs[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(Vertex &vertex, int boneID, float weight)
{
    if (weight == 0)
    {
        return;
    }

    for (int i = 0; i < MAX_BONE_PER_VERTEX; ++i)
    {
        if (vertex.boneIDs[i] < 0)
        {
            // std::cout << "boneID: " << boneID << ", weight: " << weight << std::endl;
            vertex.boneIDs[i] = boneID;
            vertex.weights[i] = weight;
            break;
        }
    }
}

void Model::extractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh *mesh)
{
    auto &boneInfoMap = m_boneInfoMap;
    int &boneCount = m_boneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = AssimpToGLM::getGLMMat4(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            // std::cout << "vertexId: " << vertexId << ", boneID: " << boneID << ", weight: " << weight << std::endl;
            assert(vertexId <= vertices.size());
            setVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}
