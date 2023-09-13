#ifndef resource_manager_hpp
#define resource_manager_hpp

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

class Model;
#include "../model/model.h"

struct TextureLoadParams
{
    std::vector<void *> data;
    int width, height, nrComponents;
    bool anisotropicFiltering;

    TextureLoadParams(std::vector<void *> data, int width, int height, int nrComponents, bool anisotropicFiltering)
        : data(data),
          width(width),
          height(height),
          nrComponents(nrComponents),
          anisotropicFiltering(anisotropicFiltering) {}
};

class ResourceManager
{
public:
    ResourceManager(std::string executablePath);
    ~ResourceManager();

    std::string m_executablePath;
    std::unordered_map<std::string, Model *> m_models;
    std::unordered_map<std::string, Texture> m_textures;

    Model *getModel(const std::string &path, bool useOffset = true);
    void disposeModel(std::string fullPath);
    Texture getTexture(const Model &model, const std::string &path, const std::string &typeName);
    void textureFromMemory(Texture &texture, const aiTexture *embeddedTexture);
    void textureFromFile(Texture &texture, const char *path, const std::string &directory);
    unsigned int textureArrayFromFile(std::vector<std::string> texturePaths, bool anisotropicFiltering = false);
    unsigned int loadTexture(TextureLoadParams params);
    unsigned int loadTextureArray(TextureLoadParams params);

private:
    void setupAnisotropicFiltering();
};

#endif /* resource_manager_hpp */
