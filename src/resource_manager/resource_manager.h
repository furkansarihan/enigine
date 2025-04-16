#ifndef resource_manager_hpp
#define resource_manager_hpp

#include <string>
#include <unordered_map>
#include <vector>

class Model;
#include "../model/model.h"

class ResourceManager
{
public:
    ResourceManager(std::string executablePath);
    ~ResourceManager();

    std::string m_executablePath;
    std::unordered_map<std::string, Model *> m_models;
    std::unordered_map<std::string, Texture *> m_textures;
    std::unordered_map<std::string, Material *> m_materials;
    std::vector<Material *> m_copyMaterials;

    Model *getModel(const std::string &path, bool isCopy = false);
    Model *getModelFullPath(const std::string &fullPath, bool isCopy = false);
    void disposeModel(std::string fullPath);
    Texture *textureFromMemory(const TextureParams &params, const std::string &key, void *buffer, unsigned int bufferSize);
    Texture *textureFromFile(const TextureParams &params, const std::string &key, const std::string &path);
    Texture *getTextureArray(std::vector<std::string> texturePaths, bool anisotropicFiltering = false);
    void loadTexture(Texture &texture, const TextureParams &params, void *data);
    void updateTexture(Texture &texture, const TextureParams &params);
    void loadTextureArray(Texture &texture, std::vector<void *> data);

private:
    void setupAnisotropicFiltering(float maxAnisotropy);
};

#endif /* resource_manager_hpp */
