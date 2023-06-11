#include "resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image/stb_image.h"

ResourceManager::ResourceManager()
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

Model *ResourceManager::getModel(const std::string &path)
{
    // TODO: sharing same model object is safe?
    if (m_models.find(path) != m_models.end())
    {
        // std::cout << "ResourceManager: found loaded model: path: " << path << std::endl;
        return m_models[path];
    }

    Model *model = new Model(this, path);
    m_models[path] = model;

    return model;
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
        texture.id = textureFromMemory(embeddedTexture);
    else
        texture.id = textureFromFile(path.c_str(), model.directory);

    m_textures[refPath] = texture;

    return texture;
}

unsigned int ResourceManager::textureFromMemory(const aiTexture *embeddedTexture)
{
    unsigned int textureID;

    int w, h, nrComponents;
    void *pData = embeddedTexture->pcData;
    unsigned int bufferSize = embeddedTexture->mWidth;

    void *data = stbi_load_from_memory((const stbi_uc *)pData, bufferSize, &w, &h, &nrComponents, 0);

    textureID = loadTexture(data, w, h, nrComponents);
    if (data)
    {
        textureID = loadTexture(data, w, h, nrComponents);
    }
    else
    {
        std::cout << "ResourceManager: texture failed to load from memory" << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

unsigned int ResourceManager::textureFromFile(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    std::cout << filename << std::endl;

    unsigned int textureID;

    int w, h, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &w, &h, &nrComponents, 0);
    if (data)
    {
        textureID = loadTexture(data, w, h, nrComponents);
    }
    else
    {
        std::cout << "ResourceManager: texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

unsigned int ResourceManager::loadTexture(void *image_data, int width, int height, int nrComponents)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    switch (nrComponents)
    {
    case 1:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, image_data);
        break;
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        break;
    default:
        std::cout << "ResourceManager: loadTexture: " << nrComponents << " component is not implemented" << std::endl;
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
