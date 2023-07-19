#ifndef resource_manager_hpp
#define resource_manager_hpp

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

class Model;
#include "../model/model.h"

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    std::unordered_map<std::string, Model *> m_models;
    std::unordered_map<std::string, Texture> m_textures;

    Model *getModel(const std::string &path, bool useOffset = true);
    Texture getTexture(const Model &model, const std::string &path, const std::string &typeName);
    unsigned int textureFromMemory(const aiTexture *embeddedTexture);
    unsigned int textureFromFile(const char *path, const std::string &directory);
    unsigned int loadTexture(void *image_data, int width, int height, int nrComponents);
};

#endif /* resource_manager_hpp */
