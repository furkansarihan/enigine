#include "model.h"

Model::Model(ResourceManager *resourceManager, std::string const &path, bool isCopyMaterial)
    : m_resourceManager(resourceManager),
      m_path(path),
      m_isCopyMaterial(isCopyMaterial)
{
    m_importer = new Assimp::Importer();

    loadModel(path);
}

Model::Model(Model *model, int meshIndex)
    : m_resourceManager(model->m_resourceManager),
      m_path(model->m_path),
      aabbMin(model->meshes[meshIndex]->aabbMin),
      aabbMax(model->meshes[meshIndex]->aabbMax),
      m_directory(model->m_directory),
      gammaCorrection(model->gammaCorrection),
      m_isCopyMaterial(model->m_isCopyMaterial)
{
    if (model->meshes.size() <= meshIndex || meshIndex < 0)
    {
        std::cout << "Model: Can't create a model from meshIndex: " << meshIndex << std::endl;
        return;
    }

    Mesh *mesh = model->meshes[meshIndex];

    if (mesh->material->blendMode == MaterialBlendMode::opaque)
        opaqueMeshes.push_back(mesh);
    else
        transmissionMeshes.push_back(mesh);
}

Model::~Model()
{
    // TODO: delete when?
    delete m_importer;

    for (int i = 0; i < meshes.size(); i++)
        delete meshes[i];
    meshes.clear();
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

void Model::updateMeshTypes()
{
    opaqueMeshes.clear();
    transmissionMeshes.clear();

    for (auto &&mesh : meshes)
    {
        if (mesh->material->blendMode == MaterialBlendMode::opaque)
            opaqueMeshes.push_back(mesh);
        else
            transmissionMeshes.push_back(mesh);
    }
}

void Model::loadModel(std::string const &path)
{
    unsigned int start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // read file via ASSIMP
    m_scene = m_importer->ReadFile(path,
                                   aiProcess_JoinIdenticalVertices |
                                       aiProcess_ImproveCacheLocality |
                                       aiProcess_OptimizeGraph |
                                       aiProcess_RemoveComponent |
                                       aiProcess_Triangulate |
                                       aiProcess_FlipUVs |
                                       aiProcess_CalcTangentSpace |
                                       aiProcess_GenBoundingBoxes);

    // check for errors
    if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode) // if is Not Zero
    {
        std::cout << "Model: ASSIMP Error: " << m_importer->GetErrorString() << std::endl;
        return;
    }

    // retrieve the directory path of the filepath
    m_directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(m_scene->mRootNode, glm::mat4(1.0));

    if (meshes.size() > 0)
    {
        aabbMin = meshes[0]->aabbMin;
        aabbMax = meshes[0]->aabbMax;
    }

    for (int i = 1; i < meshes.size(); i++)
    {
        aabbMin.x = std::min(aabbMin.x, meshes[i]->aabbMin.x);
        aabbMin.y = std::min(aabbMin.y, meshes[i]->aabbMin.y);
        aabbMin.z = std::min(aabbMin.z, meshes[i]->aabbMin.z);

        aabbMax.x = std::max(aabbMax.x, meshes[i]->aabbMax.x);
        aabbMax.y = std::max(aabbMax.y, meshes[i]->aabbMax.y);
        aabbMax.z = std::max(aabbMax.z, meshes[i]->aabbMax.z);
    }

    unsigned int end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int duration = end - start;
    std::string fileName = path.substr(path.find_last_of('/'), path.size());
    std::cout << std::setfill(' ') << std::setw(4) << duration << "ms - Model - " << fileName << std::endl;
}

void Model::processNode(aiNode *node, glm::mat4 parentTransform)
{
    glm::mat4 nodeTransform = AssimpToGLM::getGLMMat4(node->mTransformation);
    glm::mat4 transform = parentTransform * nodeTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *assimpMesh = m_scene->mMeshes[node->mMeshes[i]];

        Mesh *mesh = processMesh(assimpMesh);
        mesh->offset = transform;

        // TODO: negative scale?
        glm::vec4 aabbMin = transform * glm::vec4(mesh->aabbMin, 1.f);
        glm::vec4 aabbMax = transform * glm::vec4(mesh->aabbMax, 1.f);

        mesh->aabbMin = glm::vec3(aabbMin.x, aabbMin.y, aabbMin.z);
        mesh->aabbMax = glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z);

        meshes.push_back(mesh);
        if (mesh->material->blendMode == MaterialBlendMode::opaque)
            opaqueMeshes.push_back(mesh);
        else
            transmissionMeshes.push_back(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], transform);
}

Mesh *Model::processMesh(aiMesh *mesh)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        setVertexBoneDataToDefault(vertex);
        vertex.position = AssimpToGLM::getGLMVec3(mesh->mVertices[i]);
        if (mesh->HasNormals())
            vertex.normal = AssimpToGLM::getGLMVec3(mesh->mNormals[i]);
        // texture coordinates
        if (mesh->mTextureCoords[0])
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
        if (mesh->mTangents)
        {
            glm::vec3 vector;
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        }
        // bitangent
        if (mesh->mBitangents)
        {
            glm::vec3 vector;
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        }
        vertices.push_back(vertex);
    }
    // indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];

    Material *mat = loadMaterial(material);

    // animation
    extractBoneWeightForVertices(vertices, mesh);

    return new Mesh(mesh->mName.C_Str(),
                    vertices,
                    indices,
                    AssimpToGLM::getGLMVec3(mesh->mAABB.mMin),
                    AssimpToGLM::getGLMVec3(mesh->mAABB.mMax),
                    mat);
}

// TODO: move to resource manager
Material *Model::loadMaterial(aiMaterial *material)
{
    // return same material if exist
    std::string materialName = material->GetName().C_Str();
    if (!m_isCopyMaterial)
    {
        auto it = m_resourceManager->m_materials.find(materialName);
        if (it != m_resourceManager->m_materials.end())
            return it->second;
    }

    // TODO: merge detection for ao-rought-metal - validate unknown_map
    // TODO: transmission - thickness map
    // TODO: emissive map
    // TODO: material properties for use_ao-metal-rough map
    // TODO: opacity map - fix

    std::vector<Texture> textures;
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
    // 8. opacity maps
    std::vector<Texture> opacityMaps = loadMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity");
    textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());
    // unknown maps
    std::vector<Texture> unknownMaps = loadMaterialTextures(material, aiTextureType_UNKNOWN, "texture_unknown");
    textures.insert(textures.end(), unknownMaps.begin(), unknownMaps.end());

    Material &mat = *(new Material(materialName, textures));

    // material properties
    for (unsigned int i = 0; i < material->mNumProperties; i++)
    {
        const aiMaterialProperty *property = material->mProperties[i];
        // TODO: depends on assimp - don't
        std::string propertyName = property->mKey.C_Str();

        // TODO: enum instead of direct string?
        if (propertyName == "$clr.diffuse")
        {
            mat.albedo = *reinterpret_cast<glm::vec4 *>(property->mData);
        }
        else if (propertyName == "$mat.roughnessFactor")
        {
            mat.roughness = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.metallicFactor")
        {
            mat.metallic = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.transmission.factor")
        {
            mat.transmission = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.opacity")
        {
            mat.opacity = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$clr.emissive")
        {
            mat.emissiveColor = *reinterpret_cast<glm::vec4 *>(property->mData);
        }
        // TODO: update to assimp 5.3.1
        else if (propertyName == "$mat.emissiveIntensity")
        {
            mat.emissiveStrength = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.volume.thicknessFactor")
        {
            mat.thickness = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.refracti")
        {
            mat.ior = *reinterpret_cast<float *>(property->mData);
        }
        else if (propertyName == "$mat.blend")
        {
            // TODO:
        }
    }

    // predicted blend mode
    if (mat.transmission > 0.f || mat.opacity < 1.f)
        mat.blendMode = MaterialBlendMode::alphaBlend;

    // default parallax map properties
    if (!heightMaps.empty())
    {
        mat.parallaxMapMidLevel = 0.5;
        mat.parallaxMapScale = 0.05;
        mat.parallaxMapSampleCount = 16.0;
        mat.parallaxMapScaleMode = 1.0;
    }

    if (m_isCopyMaterial)
        m_resourceManager->m_copyMaterials.push_back(&mat);
    else
        m_resourceManager->m_materials[materialName] = &mat;

    return &mat;
}

glm::vec2 getUVScale(aiMaterial *material, aiTextureType type)
{
    glm::vec2 uvScale(1.f);

    for (unsigned int i = 0; i < material->mNumProperties; i++)
    {
        const aiMaterialProperty *property = material->mProperties[i];
        std::string propertyName = property->mKey.C_Str();

        if (propertyName == "$tex.uvtrafo" && property->mSemantic == type)
        {
            glm::vec4 uv = *reinterpret_cast<glm::vec4 *>(property->mData);
            int index = property->mIndex;
            uvScale.x = uv[2];
            uvScale.y = uv[3];

            break;
        }
    }

    return uvScale;
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string texturePath = std::string(str.C_Str());
        Texture texture;

        TextureParams textureParams;
        textureParams.generateMipmaps = true;
        textureParams.anisotropicFiltering = true;
        textureParams.maxAnisotropy = 4.f;

        const aiTexture *embeddedTexture = m_scene->GetEmbeddedTexture(str.C_Str());
        if (embeddedTexture != nullptr)
        {
            std::string key = m_path + "/" + texturePath;
            void *buffer = embeddedTexture->pcData;
            unsigned int bufferSize = embeddedTexture->mWidth;
            texture = m_resourceManager->textureFromMemory(textureParams, key, buffer, bufferSize);
        }
        else
        {
            // TODO: check relative or absolute path
            std::string path = m_directory + "/" + texturePath;
            texture = m_resourceManager->textureFromFile(textureParams, path, path);
        }

        texture.type = typeName;
        texture.uvScale = getUVScale(mat, type);
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
