#include "resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image/stb_image.h"

ResourceManager::ResourceManager(std::string executablePath)
    : m_executablePath(executablePath)
{
}

ResourceManager::~ResourceManager()
{
    for (auto &pair : m_models)
        delete pair.second;
    m_models.clear();

    for (auto &pair : m_textures)
        glDeleteTextures(1, &pair.second.id);
    m_textures.clear();

    for (auto &pair : m_materials)
        delete pair.second;
    m_materials.clear();

    for (auto &pair : m_copyMaterials)
        delete pair;
    m_copyMaterials.clear();
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

    // TODO: reuse vao option for copy model
    Model *model = new Model(this, fullPath, isCopy);
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

Texture ResourceManager::textureFromMemory(const TextureParams &params, const std::string &key,
                                           void *buffer, unsigned int bufferSize)
{
    if (m_textures.find(key) != m_textures.end())
        return m_textures[key];

    unsigned int start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    Texture texture;
    // TODO: data type?
    void *data;
    if (params.dataType == TextureDataType::UnsignedByte)
    {
        data = stbi_load_from_memory((const stbi_uc *)buffer,
                                     bufferSize,
                                     &texture.width,
                                     &texture.height,
                                     &texture.nrComponents, 0);
    }
    else
    {
        // TODO: data type?
        data = stbi_loadf_from_memory((const stbi_uc *)buffer,
                                      bufferSize,
                                      &texture.width,
                                      &texture.height,
                                      &texture.nrComponents, 0);
    }

    if (data)
    {
        loadTexture(texture, params, data);
        stbi_image_free(data);
    }
    else
        std::cout << "ResourceManager: texture failed to load from memory" << std::endl;

    unsigned int end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int duration = end - start;
    std::cout << std::setfill(' ') << std::setw(4) << duration << "ms - Texture - " << key << std::endl;

    m_textures[key] = texture;
    return texture;
}

Texture ResourceManager::textureFromFile(const TextureParams &params, const std::string &key,
                                         const std::string &path)
{
    if (m_textures.find(key) != m_textures.end())
        return m_textures[key];

    unsigned int start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    Texture texture;
    void *data;
    if (params.dataType == TextureDataType::UnsignedByte)
    {
        data = stbi_load(path.c_str(),
                         &texture.width,
                         &texture.height,
                         &texture.nrComponents, 0);
    }
    else
    {
        // TODO: data type?
        data = stbi_loadf(path.c_str(),
                          &texture.width,
                          &texture.height,
                          &texture.nrComponents, 0);
    }

    if (data)
    {
        loadTexture(texture, params, data);
        stbi_image_free(data);
    }
    else
        std::cout << "ResourceManager: texture failed to load at path: " << path << std::endl;

    unsigned int end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    unsigned int duration = end - start;
    std::string fileName = path.substr(path.find_last_of('/'), path.size());
    std::cout << std::setfill(' ') << std::setw(4) << duration << "ms - Texture - path: " << fileName << std::endl;

    m_textures[key] = texture;
    return texture;
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
    loadTextureArray(texture, tdata);
    std::cout << "ResourceManager: loaded texture: " << texture.id << std::endl;

    for (int i = 0; i < nrTextures; i++)
        stbi_image_free(tdata[i]);

    return texture;
}

// TODO: TextureParams
void ResourceManager::loadTextureArray(Texture &texture, std::vector<void *> data)
{
    int nrTextures = data.size();

    GLenum iformat;
    GLenum tformat;
    switch (texture.nrComponents)
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
        std::cout << "ResourceManager: loadTextureArray: " << texture.nrComponents << " component is not implemented" << std::endl;
    }

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture.id);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, iformat, texture.width, texture.height, nrTextures, 0, tformat, GL_UNSIGNED_BYTE, NULL);
    for (int i = 0; i < nrTextures; i++)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, texture.width, texture.height, 1, tformat, GL_UNSIGNED_BYTE, data[i]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    setupAnisotropicFiltering(4.f);
}

void ResourceManager::loadTexture(Texture &texture, const TextureParams &params, void *data)
{
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // internal format, format, data type
    unsigned int internalFormat;
    unsigned int format;
    unsigned int dataType;
    if (params.dataType == TextureDataType::UnsignedByte)
    {
        if (texture.nrComponents == 1)
        {
            internalFormat = GL_RED;
            format = GL_RED;
        }
        else if (texture.nrComponents == 2)
        {
            internalFormat = GL_RG;
            format = GL_RG;
        }
        else if (texture.nrComponents == 3)
        {
            internalFormat = GL_RGB;
            format = GL_RGB;
        }
        else if (texture.nrComponents == 4)
        {
            internalFormat = GL_RGBA;
            format = GL_RGBA;
        }
        dataType = GL_UNSIGNED_BYTE;
    }
    else
    {
        if (texture.nrComponents == 1)
        {
            if (params.dataType == TextureDataType::Float16)
                internalFormat = GL_R16F;
            if (params.dataType == TextureDataType::Float32)
                internalFormat = GL_R32F;

            format = GL_RED;
        }
        else if (texture.nrComponents == 2)
        {
            if (params.dataType == TextureDataType::Float16)
                internalFormat = GL_RG16F;
            if (params.dataType == TextureDataType::Float32)
                internalFormat = GL_RG32F;

            format = GL_RG;
        }
        else if (texture.nrComponents == 3)
        {
            if (params.dataType == TextureDataType::Float16)
                internalFormat = GL_RGB16F;
            if (params.dataType == TextureDataType::Float32)
                internalFormat = GL_RGB32F;

            format = GL_RGB;
        }
        else if (texture.nrComponents == 4)
        {
            if (params.dataType == TextureDataType::Float16)
                internalFormat = GL_RGBA16F;
            if (params.dataType == TextureDataType::Float32)
                internalFormat = GL_RGBA32F;

            format = GL_RGBA;
        }
        dataType = GL_FLOAT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texture.width, texture.height, 0, format, dataType, data);

    // min filter
    if (params.generateMipmaps)
    {
        if (params.minFilter == TextureFilterMode::Nearest)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        else if (params.minFilter == TextureFilterMode::Linear)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        if (params.minFilter == TextureFilterMode::Nearest)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else if (params.minFilter == TextureFilterMode::Linear)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // mag filter
    if (params.magFilter == TextureFilterMode::Nearest)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    else if (params.magFilter == TextureFilterMode::Linear)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

    // wrap s
    if (params.wrapModeS == TextureWrapMode::ClampToEdge)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    else if (params.wrapModeS == TextureWrapMode::ClampToBorder)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else if (params.wrapModeS == TextureWrapMode::Repeat)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    // wrap t
    if (params.wrapModeT == TextureWrapMode::ClampToEdge)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    else if (params.wrapModeT == TextureWrapMode::ClampToBorder)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    else if (params.wrapModeT == TextureWrapMode::Repeat)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (params.generateMipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    if (params.anisotropicFiltering)
        setupAnisotropicFiltering(params.maxAnisotropy);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void ResourceManager::setupAnisotropicFiltering(float maxAnisotropy)
{
    // TODO: check once?
    if (!glewIsSupported("GL_EXT_texture_filter_anisotropic"))
    {
        std::cerr << "ResourceManager: Anisotropic filtering is not supported" << std::endl;
        return;
    }

    GLfloat maxSupported;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupported);
    // std::cout << "ResourceManager: Max anisotropy supported: " << maxSupported << std::endl;

    float amount = std::min(maxAnisotropy, maxSupported);

    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, amount);
}
