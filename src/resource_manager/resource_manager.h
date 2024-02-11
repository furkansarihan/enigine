#ifndef resource_manager_hpp
#define resource_manager_hpp

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

class Model;
#include "../model/model.h"

enum class TextureDataType
{
    UnsignedByte,
    Float16,
    Float32
};

enum class TextureWrapMode
{
    ClampToEdge,
    ClampToBorder,
    Repeat,
};

enum class TextureFilterMode
{
    Nearest,
    Linear
};

struct TextureParams
{
    TextureDataType dataType;
    TextureWrapMode wrapModeS;
    TextureWrapMode wrapModeT;
    TextureFilterMode minFilter;
    TextureFilterMode magFilter;
    bool anisotropicFiltering;
    float maxAnisotropy;
    bool generateMipmaps;

    TextureParams(TextureDataType dataType = TextureDataType::UnsignedByte,
                  TextureWrapMode wrapModeS = TextureWrapMode::Repeat,
                  TextureWrapMode wrapModeT = TextureWrapMode::Repeat,
                  TextureFilterMode minFilter = TextureFilterMode::Linear,
                  TextureFilterMode magFilter = TextureFilterMode::Linear,
                  bool anisotropicFiltering = false,
                  float maxAnisotropy = 4.0f,
                  bool generateMipmaps = false)
        : dataType(dataType),
          anisotropicFiltering(anisotropicFiltering),
          wrapModeS(wrapModeS),
          wrapModeT(wrapModeT),
          minFilter(minFilter),
          magFilter(magFilter),
          maxAnisotropy(maxAnisotropy),
          generateMipmaps(generateMipmaps)
    {
    }
};

class ResourceManager
{
public:
    ResourceManager(std::string executablePath);
    ~ResourceManager();

    std::string m_executablePath;
    std::unordered_map<std::string, Model *> m_models;
    std::unordered_map<std::string, Texture> m_textures;
    std::unordered_map<std::string, Material *> m_materials;

    Model *getModel(const std::string &path, bool isCopy = false);
    Model *getModelFullPath(const std::string &fullPath, bool isCopy = false);
    void disposeModel(std::string fullPath);
    Texture textureFromMemory(const TextureParams &params, const std::string &key, void *buffer, unsigned int bufferSize);
    Texture textureFromFile(const TextureParams &params, const std::string &key, const std::string &path);
    Texture getTextureArray(std::vector<std::string> texturePaths, bool anisotropicFiltering = false);
    void loadTexture(Texture &texture, const TextureParams &params, void *data);
    void loadTextureArray(Texture &texture, std::vector<void *> data);

private:
    void setupAnisotropicFiltering(float maxAnisotropy);
};

#endif /* resource_manager_hpp */
