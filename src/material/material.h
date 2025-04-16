#pragma once

#include <string>

#include <glm/glm.hpp>
#include <vector>

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

struct Texture
{
    unsigned int id;
    int width, height, nrComponents;
    std::string type; // TODO: change
    glm::vec2 uvScale = glm::vec2(1.f);
    TextureParams params;
};

enum MaterialBlendMode
{
    opaque,
    alphaBlend,
};

class Material
{
public:
    std::string name;
    std::vector<Texture *> textures;

    MaterialBlendMode blendMode;

    glm::vec2 uvScale;
    glm::vec4 albedo;
    float roughness;
    float metallic;
    float transmission;
    float opacity;
    float ior;
    // emissive
    glm::vec4 emissiveColor;
    float emissiveStrength;
    // volume
    float thickness;

    // parallax occlusion mapping
    float parallaxMapMidLevel;
    float parallaxMapScale;
    float parallaxMapSampleCount;
    float parallaxMapScaleMode;

    Material(const std::string &name, std::vector<Texture *> &textures)
        : name(name),
          textures(textures),
          blendMode(MaterialBlendMode::opaque),
          uvScale(glm::vec2(1.f)),
          albedo(glm::vec4(1.f)),
          metallic(0.f),
          roughness(0.f),
          transmission(0.f),
          opacity(1.f),
          ior(1.45f),
          emissiveColor(glm::vec4(0.f)),
          emissiveStrength(0.f),
          thickness(0.f),
          parallaxMapMidLevel(0.f),
          parallaxMapScale(0.f),
          parallaxMapSampleCount(0.f),
          parallaxMapScaleMode(0.f)
    {
    }
};
