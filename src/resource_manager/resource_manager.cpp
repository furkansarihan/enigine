#include "resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image/stb_image.h"

ResourceManager::ResourceManager(std::string executablePath)
    : m_executablePath(executablePath)
{
}

ResourceManager::~ResourceManager()
{
    // TODO: destruction
    for (auto &pair : m_models)
        delete pair.second;
    m_models.clear();

    for (auto &pair : m_textures)
        glDeleteTextures(1, &pair.second.id);
    m_textures.clear();
}

Model *ResourceManager::getModel(const std::string &path, bool isCopy)
{
    std::string fullPath = m_executablePath + '/' + path;
    return getModelFullPath(fullPath, isCopy);
}

Model *ResourceManager::getModelFullPath(const std::string &fullPath, bool isCopy)
{
    // TODO: sharing same model object is safe?
    if (!isCopy && m_models.find(fullPath) != m_models.end())
    {
        // std::cout << "ResourceManager: found loaded model: path: " << path << std::endl;
        return m_models[fullPath];
    }

    Model *model = new Model(this, fullPath);
    if (!isCopy)
        m_models[fullPath] = model;

    return model;
}

void ResourceManager::disposeModel(std::string fullPath)
{
    if (m_models.find(fullPath) == m_models.end())
        return;

    delete m_models[fullPath];
    m_models.erase(fullPath);
}

Texture ResourceManager::getTexture(const Model &model, const std::string &path, const std::string &typeName)
{
    const aiTexture *embeddedTexture = model.m_scene->GetEmbeddedTexture(path.c_str());

    // embedded texture path
    std::string refPath = path;
    if (embeddedTexture != nullptr)
        refPath = model.m_path + path;

    if (m_textures.find(refPath) != m_textures.end())
    {
        // std::cout << "ResourceManager: found loaded texture: path: " << path << std::endl;
        return m_textures[refPath];
    }

    Texture texture;
    texture.type = typeName;

    if (embeddedTexture != nullptr)
        textureFromMemory(texture, embeddedTexture);
    else
        textureFromFile(texture, path.c_str(), model.directory);

    m_textures[refPath] = texture;

    return texture;
}

void ResourceManager::textureFromMemory(Texture &texture, const aiTexture *embeddedTexture)
{
    int w, h, nrComponents;
    void *pData = embeddedTexture->pcData;
    unsigned int bufferSize = embeddedTexture->mWidth;

    void *data = stbi_load_from_memory((const stbi_uc *)pData, bufferSize, &w, &h, &nrComponents, 0);
    if (data)
    {
        std::vector<void *> tdata;
        tdata.push_back(data);
        texture.id = loadTexture(TextureLoadParams(tdata, w, h, nrComponents, false));
        texture.nrComponents = nrComponents;
        texture.width = w;
        texture.height = h;
    }
    else
    {
        std::cout << "ResourceManager: texture failed to load from memory" << std::endl;
    }

    stbi_image_free(data);
}

void ResourceManager::textureFromFile(Texture &texture, const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    std::cout << filename << std::endl;

    int w, h, nrComponents;
    void *data = stbi_load(filename.c_str(), &w, &h, &nrComponents, 0);
    if (data)
    {
        std::vector<void *> tdata;
        tdata.push_back(data);
        texture.id = loadTexture(TextureLoadParams(tdata, w, h, nrComponents, false));
        texture.nrComponents = nrComponents;
        texture.width = w;
        texture.height = h;
    }
    else
    {
        std::cout << "ResourceManager: texture failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);
}

Texture ResourceManager::getTextureArray(std::vector<std::string> texturePaths, bool anisotropicFiltering)
{
    Texture texture;
    int nrTextures = texturePaths.size();
    int twidth[nrTextures];
    int theight[nrTextures];
    int tnrComponents[nrTextures];

    std::vector<void *> tdata;

    for (int i = 0; i < nrTextures; i++)
        tdata.push_back(stbi_load((m_executablePath + '/' + texturePaths[i]).c_str(), &twidth[i], &theight[i], &tnrComponents[i], 0));

    for (int i = 0; i < nrTextures; i++)
    {
        if (tdata[i] == nullptr)
        {
            std::cout << "ResourceManager: textureArrayFromFile: can not read the texture at: " << texturePaths[i] << std::endl;
            return texture; // TODO: correct? - could be default failed texture
        }
    }

    // TODO: check size and nrComponents consistency

    float tWidth = twidth[0];
    float tHeight = theight[0];
    int tNrComponents = tnrComponents[0];

    texture.width = tWidth;
    texture.height = tHeight;
    texture.nrComponents = tNrComponents;
    texture.id = loadTextureArray(TextureLoadParams(tdata, tWidth, tHeight, tNrComponents, anisotropicFiltering));
    std::cout << "ResourceManager: loaded texture: " << texture.id << std::endl;

    for (int i = 0; i < nrTextures; i++)
        stbi_image_free(tdata[i]);

    return texture;
}

unsigned int ResourceManager::loadTextureArray(TextureLoadParams params)
{
    int nrTextures = params.data.size();

    GLenum iformat;
    GLenum tformat;
    switch (params.nrComponents)
    {
    case 1:
        iformat = GL_R8;
        tformat = GL_RED;
        break;
    case 2:
        iformat = GL_RG8;
        tformat = GL_RG;
    case 3:
        iformat = GL_RGB8;
        tformat = GL_RGB;
        break;
    case 4:
        iformat = GL_RGBA16F;
        tformat = GL_RGBA;
        break;
    default:
        std::cout << "ResourceManager: loadTextureArray: " << params.nrComponents << " component is not implemented" << std::endl;
    }

    unsigned int textureArrayId = 0;
    glGenTextures(1, &textureArrayId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, iformat, params.width, params.height, nrTextures, 0, tformat, GL_UNSIGNED_BYTE, NULL);
    for (int i = 0; i < nrTextures; i++)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, params.width, params.height, 1, tformat, GL_UNSIGNED_BYTE, params.data[i]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    if (params.anisotropicFiltering)
        setupAnisotropicFiltering();

    return textureArrayId;
}

unsigned int ResourceManager::loadTexture(TextureLoadParams params)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    switch (params.nrComponents)
    {
    case 1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, params.width, params.height, 0, GL_RED, GL_UNSIGNED_BYTE, params.data[0]);
        break;
    case 2:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, params.width, params.height, 0, GL_RG, GL_UNSIGNED_BYTE, params.data[0]);
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, params.width, params.height, 0, GL_RGB, GL_UNSIGNED_BYTE, params.data[0]);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, params.width, params.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, params.data[0]);
        break;
    default:
        std::cout << "ResourceManager: loadTexture: " << params.nrComponents << " component is not implemented" << std::endl;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void ResourceManager::setupAnisotropicFiltering()
{
    if (!glewIsSupported("GL_EXT_texture_filter_anisotropic"))
    {
        std::cerr << "ResourceManager: Anisotropic filtering is not supported" << std::endl;
        return;
    }

    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    // std::cout << "ResourceManager: Max anisotropy supported: " << maxAnisotropy << std::endl;

    float amount = std::min(4.0f, maxAnisotropy);

    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, amount);
}
