#include "terrain.h"

#include "../external/stb_image/stb_image.h"

// TODO: change asset path at runtime
Terrain::Terrain(PbrManager *pbrManager, PhysicsWorld *physicsWorld, const std::string &filename, float minHeight, float maxHeight, float scaleHoriz, bool PBR)
    : m_pbrManager(pbrManager),
      m_minHeight(minHeight),
      m_maxHeight(maxHeight),
      m_scaleHoriz(scaleHoriz),
      m_PBR(PBR)
{
    // TODO: keep track initialization state

    // int n = 255;
    int m = resolution; // m = (n+1)/4

    // create mxm
    unsigned int vbo_mxm, ebo_mxm;
    createMesh(m, m, vbo_mxm, vao_mxm, ebo_mxm);
    // create 3xm
    unsigned int vbo_3xm, ebo_3xm;
    createMesh(3, m, vbo_3xm, vao_3xm, ebo_3xm);
    // TODO: 1 vao for vbo_2m1x2 and vbo_2x2m1 - rotate - mirror
    // create (2m + 1)x2
    unsigned int vbo_2m1x2, ebo_2m1x2;
    createMesh(2 * m + 1, 2, vbo_2m1x2, vao_2m1x2, ebo_2m1x2);
    // create 2x(2m + 1)
    unsigned int vbo_2x2m1, ebo_2x2m1;
    createMesh(2, 2 * m + 1, vbo_2x2m1, vao_2x2m1, ebo_2x2m1);
    // create outer degenerate triangles
    unsigned int vbo_0, ebo_0;
    createOuterCoverMesh(4 * (m - 1) + 2, vbo_0, vao_0, ebo_0);
    // triangle fan - outside of terrain
    unsigned int vbo_tf, ebo_tf;
    createTriangleFanMesh(m - 1, vbo_tf, vao_tf, ebo_tf);

    // 3x3 - finer center
    unsigned int vbo_3x3, ebo_3x3;
    createMesh(3, 3, vbo_3x3, vao_3x3, ebo_3x3);

    // buffer elevationSampler texture
    glGenTextures(1, &textureID);
    int nrComponents;
    data = stbi_loadf(filename.c_str(), &heightmapWidth, &heightmapHeight, &nrComponents, 1);
    if (data == nullptr)
    {
        fprintf(stderr, "Failed to read heightmap\n");
        return;
    }

    std::cout << "heightmapWidth: " << heightmapWidth << std::endl;
    std::cout << "heightmapHeight: " << heightmapHeight << std::endl;
    std::cout << "nrComponents: " << nrComponents << std::endl;

    if (nrComponents != 1)
    {
        fprintf(stderr, "Failed to initialize heightmap. Number of components of the texture must be 1.\n");
        return;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    // TODO: variable internal format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, heightmapWidth, heightmapHeight, 0, GL_RED, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Physics
    terrainBody = physicsWorld->createTerrain(
        heightmapWidth,
        heightmapHeight,
        data,
        0,
        1,
        1,
        false,
        btVector3(0, 0, 0));

    terrainBody->setCollisionFlags(btRigidBody::CF_DISABLE_VISUALIZE_OBJECT |
                                   btCollisionObject::CF_STATIC_OBJECT);

    updateHorizontalScale();

    // texture arrays
    if (m_PBR)
    {
        // TODO: same order with fragment shader - samplerNames
        std::string textureTypes[] = {
            "albedo",
            "ao",
            "metallic",
            "normal",
            "rough",
        };
        std::string fileTextenstions[] = {
            ".jpg",
            ".jpg",
            ".jpg",
            ".png",
            ".jpg",
        };
        for (int i = 0; i < 5; i++)
        {
            PBRTextureArrayIds[i] = 0;
            std::string texturePaths[6] = {
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/water" + fileTextenstions[i],
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/sand" + fileTextenstions[i],
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/rock" + fileTextenstions[i],
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/grass" + fileTextenstions[i],
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/stone" + fileTextenstions[i],
                "../src/assets/terrain/pbr-texture-2/" + textureTypes[i] + "/snow" + fileTextenstions[i],
            };
            initTextureArray(PBRTextureArrayIds[i], texturePaths);
        }
    }
    else
    {
        std::string texturePaths[6] = {
            "../src/assets/images/water-1.jpg",
            "../src/assets/images/sand-1.jpg",
            "../src/assets/images/stone-1.jpg",
            "../src/assets/images/grass-1.jpg",
            "../src/assets/images/rock-1.jpg",
            "../src/assets/images/snow-1.jpg",
        };
        initTextureArray(normalTextureArrayId, texturePaths);
    }
}

Terrain::~Terrain()
{
    stbi_image_free(data);
}

void Terrain::initTextureArray(unsigned int &textureArrayId, std::string *texturePaths)
{
    glGenTextures(1, &textureArrayId);
    int twidth[6];
    int theight[6];
    int tnrComponents[6];
    int nrTextures = 6;

    unsigned char *tdata0 = stbi_load(texturePaths[0].c_str(), &twidth[0], &theight[0], &tnrComponents[0], 0);
    unsigned char *tdata1 = stbi_load(texturePaths[1].c_str(), &twidth[1], &theight[1], &tnrComponents[1], 0);
    unsigned char *tdata2 = stbi_load(texturePaths[2].c_str(), &twidth[2], &theight[2], &tnrComponents[2], 0);
    unsigned char *tdata3 = stbi_load(texturePaths[3].c_str(), &twidth[3], &theight[3], &tnrComponents[3], 0);
    unsigned char *tdata4 = stbi_load(texturePaths[4].c_str(), &twidth[4], &theight[4], &tnrComponents[4], 0);
    unsigned char *tdata5 = stbi_load(texturePaths[5].c_str(), &twidth[5], &theight[5], &tnrComponents[5], 0);

    if (tdata0 == nullptr || tdata1 == nullptr || tdata2 == nullptr || tdata3 == nullptr || tdata4 == nullptr || tdata5 == nullptr)
    {
        fprintf(stderr, "Failed to read textures\n");
        return;
    }

    for (int i = 0; i < 6; i++)
        std::cout << "path: " << texturePaths[i] << ", w: " << twidth[i] << ", h: " << theight[i] << ", comp: " << tnrComponents[i] << std::endl;

    float tWidth = twidth[0];
    float tHeight = theight[0];

    GLenum iformat;
    GLenum tformat;
    if (tnrComponents[0] == 1)
    {
        iformat = GL_R8;
        tformat = GL_RED;
    }
    else if (tnrComponents[0] == 3)
    {
        iformat = GL_RGB8;
        tformat = GL_RGB;
    }
    else if (tnrComponents[0] == 4)
    {
        iformat = GL_RGBA16F;
        tformat = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, iformat, tWidth, tHeight, nrTextures, 0, tformat, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata0);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata1);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata2);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata3);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata4);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, tWidth, tHeight, 1, tformat, GL_UNSIGNED_BYTE, tdata5);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    setupAnisotropicFiltering();

    stbi_image_free(tdata0);
    stbi_image_free(tdata1);
    stbi_image_free(tdata2);
    stbi_image_free(tdata3);
    stbi_image_free(tdata4);
    stbi_image_free(tdata5);
}

void Terrain::setupAnisotropicFiltering()
{
    if (!glewIsSupported("GL_EXT_texture_filter_anisotropic"))
    {
        std::cerr << "Anisotropic filtering is not supported" << std::endl;
        return;
    }

    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    std::cout << "Max anisotropy supported: " << maxAnisotropy << std::endl;

    float amount = std::min(4.0f, maxAnisotropy);

    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, amount);
}

// https://stackoverflow.com/a/9194117/11601515
// multiple is a power of 2
int Terrain::roundUp(int numToRound, int multiple)
{
    assert(multiple && ((multiple & (multiple - 1)) == 0));
    return (numToRound + multiple - 1) & -multiple;
}

float Terrain::roundUpf(float numToRound, float multiple)
{
    float remainder = fmod(numToRound, multiple);
    if (remainder == 0.0f)
    {
        return numToRound;
    }
    return numToRound + multiple - remainder;
}

void Terrain::createTriangleFanMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    std::vector<float> vertices;
    std::vector<int> indices;
    // --
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(i);
        vertices.push_back(0);
    }
    // --|
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(size);
        vertices.push_back(i);
    }
    // --|
    // --|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(i);
        vertices.push_back(size);
    }
    // |--|
    // |--|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(0);
        vertices.push_back(i);
    }

    // center
    vertices.push_back((float)size / 2);
    vertices.push_back((float)size / 2);

    int half = vertices.size() / 2;
    int centerIndex = half - 1;
    for (int i = 0; i < half - 2; i++)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(centerIndex);
    }
    indices.push_back(half - 2);
    indices.push_back(0);
    indices.push_back(centerIndex);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    std::vector<float> vertices;
    std::vector<int> indices;
    // --
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(i);
        vertices.push_back(0);
    }
    // --|
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(size);
        vertices.push_back(i);
    }
    // --|
    // --|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(i);
        vertices.push_back(size);
    }
    // |--|
    // |--|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(0);
        vertices.push_back(i);
    }
    int half = vertices.size() / 2;
    for (int i = 0; i <= half - 2; i += 2)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
        if (i != half - 2)
        {
            indices.push_back(i + 2);
        }
        else
        {
            indices.push_back(0);
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    std::vector<float> vertices;
    std::vector<int> indices;

    for (int i = 0; i < m; i++)
    {
        // vertices
        for (int j = 0; j < n; j++)
        {
            vertices.push_back(i);
            vertices.push_back(j);
        }
        // indices
        if (i == m - 1)
        {
            break;
        }
        int length = n;
        for (int t = 0; t < n - 1; t++)
        {
            int start = n * i + t;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + length);
            indices.push_back(start + 1);
            indices.push_back(start + length);
            indices.push_back(start + length + 1);
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::drawColor(Shader terrainShader, glm::vec3 lightPosition, glm::vec3 lightColor, float lightPower,
                        glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos,
                        glm::mat4 cullView, glm::mat4 cullProjection, glm::vec3 cullViewPos,
                        GLuint shadowmapId, glm::vec3 camPos, glm::vec3 camView, glm::vec4 frustumDistances,
                        bool ortho)
{
    calculatePlanes(cullProjection, cullView);

    terrainShader.use();
    terrainShader.setFloat("scaleHoriz", m_scaleHoriz);
    terrainShader.setFloat("minHeight", m_minHeight);
    terrainShader.setFloat("maxHeight", m_maxHeight);
    terrainShader.setVec3("viewerPos", viewPos);
    terrainShader.setVec3("lightDirection", lightPosition);
    terrainShader.setVec3("lightColor", lightColor);
    terrainShader.setFloat("lightPower", lightPower);
    terrainShader.setVec2("uvOffset", uvOffset);
    terrainShader.setVec2("terrainSize", glm::vec2(width, height));
    terrainShader.setFloat("fogMaxDist", fogMaxDist);
    terrainShader.setFloat("fogMinDist", fogMinDist);
    terrainShader.setVec4("fogColor", fogColor);
    terrainShader.setFloat("ambientMult", ambientMult);
    terrainShader.setFloat("diffuseMult", diffuseMult);
    terrainShader.setFloat("specularMult", specularMult);

    // shadowmap
    terrainShader.setMat4("worldViewProjMatrix", projection * view);
    terrainShader.setMat4("M", glm::mat4(1.0f));
    terrainShader.setMat4("V", view);

    terrainShader.setVec3("CamPos", camPos);
    terrainShader.setVec3("CamView", camView);
    terrainShader.setVec4("FrustumDistances", frustumDistances);
    terrainShader.setBool("ShowCascade", showCascade);
    terrainShader.setVec3("Bias", shadowBias);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(terrainShader.id, "elevationSampler"), 0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glActiveTexture(GL_TEXTURE0 + 1);
    glUniform1i(glGetUniformLocation(terrainShader.id, "ShadowMap"), 1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmapId);

    if (m_PBR)
    {
        std::string samplerNames[] = {
            "texture_diffuse1",
            "texture_ao1",
            "texture_metal1",
            "texture_normal1",
            "texture_rough1",
        };
        for (int i = 0; i < 5; i++)
        {
            glActiveTexture(GL_TEXTURE0 + 2 + i);
            glUniform1i(glGetUniformLocation(terrainShader.id, samplerNames[i].c_str()), 2 + i);
            glBindTexture(GL_TEXTURE_2D_ARRAY, PBRTextureArrayIds[i]);
        }

        glActiveTexture(GL_TEXTURE0 + 7);
        glUniform1i(glGetUniformLocation(terrainShader.id, "irradianceMap"), 7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->irradianceMap);

        glActiveTexture(GL_TEXTURE0 + 8);
        glUniform1i(glGetUniformLocation(terrainShader.id, "prefilterMap"), 8);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_pbrManager->prefilterMap);

        glActiveTexture(GL_TEXTURE0 + 9);
        glUniform1i(glGetUniformLocation(terrainShader.id, "brdfLUT"), 9);
        glBindTexture(GL_TEXTURE_2D, m_pbrManager->brdfLUTTexture);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0 + 2);
        glUniform1i(glGetUniformLocation(terrainShader.id, "textureSampler"), 2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, normalTextureArrayId);
    }

    this->draw(terrainShader, cullViewPos, ortho);
}

void Terrain::updateHorizontalScale()
{
    width = heightmapWidth * m_scaleHoriz;
    height = heightmapHeight * m_scaleHoriz;
    w = 1.0 / heightmapWidth;
    h = 1.0 / heightmapHeight;

    float scaleFactor = m_maxHeight - m_minHeight;
    terrainBody->getWorldTransform().setOrigin(btVector3(width / 2.0, scaleFactor / 2.0 + m_minHeight, height / 2.0));
    terrainBody->getCollisionShape()->setLocalScaling(btVector3(m_scaleHoriz, scaleFactor, m_scaleHoriz));

    updateHeightMatrix();
}

void Terrain::drawDepth(Shader terrainShadow, glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos)
{
    calculatePlanes(projection, view);

    terrainShadow.use();
    terrainShadow.setFloat("scaleHoriz", m_scaleHoriz);
    terrainShadow.setFloat("minHeight", m_minHeight);
    terrainShadow.setFloat("maxHeight", m_maxHeight);
    terrainShadow.setVec2("uvOffset", uvOffset);
    terrainShadow.setVec2("terrainSize", glm::vec2(width, height));

    terrainShadow.setMat4("worldViewProjMatrix", projection * view);
    terrainShadow.setMat4("M", glm::mat4(1.0f));
    terrainShadow.setMat4("V", view);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(terrainShadow.id, "elevationSampler"), 0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    this->draw(terrainShadow, viewPos, true);
}

void Terrain::draw(Shader terrainShader, glm::vec3 viewPos, bool ortho)
{
    int m = resolution;

    // for each level
    int lastRoundX, lastRoundZ = 0;
    for (int i = 1; i < level; i++)
    {
        // set param for each footprint
        int scale = pow(2, i - 1);

        int X = -1 * (2 * m * scale) + (int)viewPos.x + (int)terrainCenter.x;
        int Z = -1 * (2 * m * scale) + (int)viewPos.z + (int)terrainCenter.z;

        int x = roundUp(X, scale * 2);
        int z = roundUp(Z, scale * 2);

        if (i % 3 == 0)
        {
            terrainShader.setVec3("wireColor", glm::vec3(1, 0.522, 0.522));
        }
        else if (i % 3 == 1)
        {
            terrainShader.setVec3("wireColor", glm::vec3(0.522, 1, 0.682));
        }
        else
        {
            terrainShader.setVec3("wireColor", glm::vec3(0.522, 0.827, 1));
        }

        if (wireframe)
        {
            terrainShader.setBool("wireframe", true);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            terrainShader.setBool("wireframe", false);
        }

        int sizeMM = scale * (m - 1);
        int size2 = scale * 2;

        // mxm
        int mmIndices = m * (m - 1) * 6;

        glBindVertexArray(vao_mxm);

        glm::vec2 positions_mxm[] = {
            glm::vec2(x, z),
            glm::vec2(x + sizeMM, z),
            glm::vec2(x + sizeMM * 2 + size2, z),
            glm::vec2(x + sizeMM * 3 + size2, z),
            glm::vec2(x + sizeMM * 3 + size2, z + sizeMM),
            glm::vec2(x + sizeMM * 3 + size2, z + sizeMM * 2 + size2),
            glm::vec2(x + sizeMM * 3 + size2, z + sizeMM * 3 + size2),
            glm::vec2(x + sizeMM * 2 + size2, z + sizeMM * 3 + size2),
            glm::vec2(x + sizeMM, z + sizeMM * 3 + size2),
            glm::vec2(x, z + sizeMM * 3 + size2),
            glm::vec2(x, z + sizeMM * 2 + size2),
            glm::vec2(x, z + sizeMM),
            // fine level
            glm::vec2(x + sizeMM, z + sizeMM),
            glm::vec2(x + 2 * sizeMM + size2, z + sizeMM),
            glm::vec2(x + sizeMM, z + 2 * sizeMM + size2),
            glm::vec2(x + 2 * sizeMM + size2, z + 2 * sizeMM + size2),
        };

        for (int i = 0; i < 12; i++)
        {
            glm::vec2 pos = positions_mxm[i];
            drawBlock(terrainShader, vao_mxm, scale, glm::vec2(m, m), pos, mmIndices, viewPos, ortho);
        }

        // fine level
        if (i == 1)
        {
            for (int i = 12; i < 16; i++)
            {
                glm::vec2 pos = positions_mxm[i];
                drawBlock(terrainShader, vao_mxm, scale, glm::vec2(m, m), pos, mmIndices, viewPos, ortho);
            }

            // TODO: center
            glBindVertexArray(vao_3x3);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM, z + 2 * sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM), h * (z + 2 * sizeMM)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);
        }

        // 3xm
        int indices3M = 2 * (m - 1) * 6;

        glBindVertexArray(vao_3xm);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * z));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 3 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 3 + size2)));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)M_PI_2, glm::vec3(0, 1, 0));
        model = glm::rotate(model, (float)M_PI, glm::vec3(1, 0, 0));
        terrainShader.setMat4("M", model);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 2 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 2 + size2)));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 4 + size2, z + sizeMM * 2 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2 + size2)));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        terrainShader.setMat4("M", glm::mat4(1.0f));

        // fine level 3xm
        if (i == 1)
        {
            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)M_PI_2, glm::vec3(0, 1, 0));
            model = glm::rotate(model, (float)M_PI, glm::vec3(1, 0, 0));
            terrainShader.setMat4("M", model);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setMat4("M", glm::mat4(1.0f));
        }

        if (i != 1)
        {
            // 2m1x2
            int indices212 = 2 * m * 6;

            glBindVertexArray(vao_2m1x2);

            if (lastRoundZ == z + sizeMM)
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 3 + size2 - scale));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 3 + size2 - scale)));
            }
            else
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
            }
            glDrawElements(GL_TRIANGLES, indices212, GL_UNSIGNED_INT, 0);

            glBindVertexArray(vao_2x2m1);
            if (lastRoundX == x + sizeMM)
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2 - scale, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (z + sizeMM * 3 + size2 - scale), h * (x + sizeMM)));
            }
            else
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
            }
            glDrawElements(GL_TRIANGLES, indices212, GL_UNSIGNED_INT, 0);
        }

        // outer degenerate triangles
        int indicesOuter = 4 * (m - 1) * 3 * 4;

        terrainShader.setVec3("wireColor", glm::vec3(0, 1, 1));
        glBindVertexArray(vao_0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, x, z));
        glDrawElements(GL_TRIANGLES, indicesOuter, GL_UNSIGNED_INT, 0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        lastRoundX = x;
        lastRoundZ = z;
    }
}

void Terrain::drawInstance(glm::vec3 grassColorFactor, glm::vec3 playerPos, Shader instanceShader, Model *model, int tileSize, float density, glm::mat4 projection, glm::mat4 view, glm::vec3 viewPos)
{
    int tileSize1 = tileSize;
    int tileSize2 = tileSize * 2;
    int tileSize3 = tileSize * 3;

    glm::vec2 topLeft1(-tileSize1, -tileSize1);
    glm::vec2 topLeft2(-1 * (tileSize1 + tileSize2), -1 * (tileSize1 + tileSize2));
    glm::vec2 topLeft3(-1 * (tileSize1 + tileSize2 + tileSize3), -1 * (tileSize1 + tileSize2 + tileSize3));

    // length of each instance
    int columnCount = tileSize1 * density;
    float mult = (float)tileSize1 / columnCount;

    float x = roundUpf(viewPos.x, mult);
    float z = roundUpf(viewPos.z, mult);

    // TODO: variable level
    glm::vec2 positions[] = {
        // level 1
        glm::vec2(x, z) + topLeft1,
        glm::vec2(x, z) + topLeft1 + glm::vec2(tileSize1, 0),
        glm::vec2(x, z) + topLeft1 + glm::vec2(0, tileSize1),
        glm::vec2(x, z) + topLeft1 + glm::vec2(tileSize1, tileSize1),
        // level 2
        glm::vec2(x, z) + topLeft2,
        glm::vec2(x, z) + topLeft2 + glm::vec2(tileSize2, 0),
        glm::vec2(x, z) + topLeft2 + glm::vec2(tileSize2 * 2, 0),
        // level 2 - 2
        glm::vec2(x, z) + topLeft2 + glm::vec2(0, tileSize2),
        glm::vec2(x, z) + topLeft2 + glm::vec2(tileSize2 * 2, tileSize2),
        // level 2 - 3
        glm::vec2(x, z) + topLeft2 + glm::vec2(0, tileSize2 * 2),
        glm::vec2(x, z) + topLeft2 + glm::vec2(tileSize2, tileSize2 * 2),
        glm::vec2(x, z) + topLeft2 + glm::vec2(tileSize2 * 2, tileSize2 * 2),
        // level 3 - 1
        glm::vec2(x, z) + topLeft3,
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3, 0),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 2, 0),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 3, 0),
        // level 3 - 2, 3
        glm::vec2(x, z) + topLeft3 + glm::vec2(0, tileSize3),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 3, tileSize3),
        glm::vec2(x, z) + topLeft3 + glm::vec2(0, tileSize3 * 2),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 3, tileSize3 * 2),
        // level 3 - 4
        glm::vec2(x, z) + topLeft3 + glm::vec2(0, tileSize3 * 3),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3, tileSize3 * 3),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 2, tileSize3 * 3),
        glm::vec2(x, z) + topLeft3 + glm::vec2(tileSize3 * 3, tileSize3 * 3),
    };

    instanceShader.use();
    instanceShader.setMat4("projection", projection);
    instanceShader.setMat4("view", view);
    instanceShader.setVec2("elevationMapSize", glm::vec2(w, h));
    instanceShader.setFloat("scaleHoriz", m_scaleHoriz);
    instanceShader.setFloat("minHeight", m_minHeight);
    instanceShader.setFloat("maxHeight", m_maxHeight);
    instanceShader.setFloat("u_time", (float)glfwGetTime());

    instanceShader.setVec3("playerPos", playerPos);
    instanceShader.setVec3("grassColorFactor", grassColorFactor);
    instanceShader.setFloat("windIntensity", m_windIntensity);
    instanceShader.setFloat("mult", mult);

    // TODO: texture unit based on texture count for instanced model
    glActiveTexture(GL_TEXTURE0 + 1);
    glUniform1i(glGetUniformLocation(instanceShader.id, "elevationSampler"), 1);
    glBindTexture(GL_TEXTURE_2D, textureID);

    for (int i = 0; i < 24; i++)
    {
        glm::vec2 pos = positions[i];
        int columnSize = i < 4 ? tileSize1 : i < 12 ? tileSize2
                                                    : tileSize3;
        int columnCount = columnSize / mult + 0.01;
        int instanceCount = columnCount * columnCount;

        instanceShader.setInt("columnCount", columnCount);
        instanceShader.setVec2("referencePos", pos);

        glm::vec2 blockSize(columnSize, columnSize);

        glm::vec2 topLeft = pos;
        glm::vec2 topRight = pos + glm::vec2(blockSize.x, 0);
        glm::vec2 bottomLeft = pos + glm::vec2(0, blockSize.y);
        glm::vec2 bottomRight = pos + blockSize;

        if (inFrustum(topLeft, topRight, bottomLeft, bottomRight, viewPos, false))
        {
            model->drawInstanced(instanceShader, instanceCount);
        }
    }
}

// TODO: reduce glBindVertexArray calls
void Terrain::drawBlock(Shader shader, unsigned int vao, int scale, glm::vec2 size, glm::vec2 pos, int indiceCount, glm::vec3 viewPos, bool ortho)
{
    glm::vec2 blockSize = glm::vec2(size.x * scale, size.y * scale);

    glm::vec2 topLeft = pos;
    glm::vec2 topRight = pos + glm::vec2(blockSize.x, 0);
    glm::vec2 bottomLeft = pos + glm::vec2(0, blockSize.y);
    glm::vec2 bottomRight = pos + blockSize;

    // frustum culling
    if (!inFrustum(topLeft, topRight, bottomLeft, bottomRight, viewPos, ortho))
    {
        markHeightRange(topLeft, bottomRight, true);
        return;
    }

    // outside of terrain
    bool outside = false;
    if (bottomRight.y < 0 || topLeft.y > height || bottomRight.x < 0 || topLeft.x > width)
    {
        outside = true;
        glBindVertexArray(vao_tf);
    }

    shader.setVec4("scaleFactor", glm::vec4(scale, scale, pos.x, pos.y));
    shader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * pos.x, h * pos.y));
    glDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_INT, 0);

    if (outside)
    {
        glBindVertexArray(vao);
    }

    markHeightRange(topLeft, bottomRight, outside);
}

bool Terrain::inFrustum(glm::vec2 topLeft, glm::vec2 topRight, glm::vec2 bottomLeft, glm::vec2 bottomRight, glm::vec3 viewPos, bool ortho)
{
    HeightCell range = getHeightRange(topLeft, bottomRight);
    float scale = m_maxHeight - m_minHeight;
    float minHeight = range.min * scale * m_cellScaleMult;
    float maxHeight = range.max * scale * m_cellScaleMult;
    glm::vec3 corners[] = {
        glm::vec3(topLeft.x, minHeight, topLeft.y),
        glm::vec3(topRight.x, minHeight, topRight.y),
        glm::vec3(bottomLeft.x, minHeight, bottomLeft.y),
        glm::vec3(bottomRight.x, minHeight, bottomRight.y),
        glm::vec3(topLeft.x, maxHeight, topLeft.y),
        glm::vec3(topRight.x, maxHeight, topRight.y),
        glm::vec3(bottomLeft.x, maxHeight, bottomLeft.y),
        glm::vec3(bottomRight.x, maxHeight, bottomRight.y),
    };

    for (int i = 0; i < 5; i++)
    {
        if (!inFrontOf(m_planes[i], corners, viewPos, ortho))
        {
            return false;
        }
    }

    return true;
}

bool Terrain::inFrontOf(glm::vec4 plane, glm::vec3 corners[8], glm::vec3 viewPos, bool ortho)
{
    glm::vec3 planeNormal(plane.x, plane.y, plane.z);

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 pos = corners[i] - viewPos;
        float res = glm::dot(planeNormal, pos);

        // TODO: why -1 works for ortho?
        if (res > (ortho ? -1 : 0))
        {
            return true;
        }
    }

    return false;
}

HeightCell Terrain::getHeightRange(glm::vec2 topLeft, glm::vec2 bottomRight)
{
    glm::vec2 topLeftIndex = getCellIndex(topLeft);
    glm::vec2 bottomRightIndex = getCellIndex(bottomRight);

    float minHeight = 1.f;
    float maxHeight = 0.f;

    for (int x = topLeftIndex.x; x <= bottomRightIndex.x; x++)
    {
        if (x < 0 || x > m_horizontalCellCount - 1)
            continue;

        for (int y = topLeftIndex.y; y <= bottomRightIndex.y; y++)
        {
            if (y < 0 || y > m_verticalCellCount - 1)
                continue;

            float min = m_heightMatrix[x][y].min;
            float max = m_heightMatrix[x][y].max;

            if (min < minHeight)
                minHeight = min;
            if (max > maxHeight)
                maxHeight = max;
        }
    }

    return HeightCell(minHeight, maxHeight);
}

// debug
void Terrain::markHeightRange(glm::vec2 topLeft, glm::vec2 bottomRight, bool culled)
{
    if (!m_drawHeightCells)
        return;

    glm::vec2 topLeftIndex = getCellIndex(topLeft);
    glm::vec2 bottomRightIndex = getCellIndex(bottomRight);

    for (int x = topLeftIndex.x; x <= bottomRightIndex.x; x++)
    {
        if (x < 0 || x > m_horizontalCellCount - 1)
            continue;

        for (int y = topLeftIndex.y; y <= bottomRightIndex.y; y++)
        {
            if (y < 0 || y > m_verticalCellCount - 1)
                continue;

            m_heightMatrixCulled[x][y] = culled;
        }
    }
}

glm::vec2 Terrain::getCellIndex(glm::vec2 point)
{
    glm::vec2 index;
    index.x = static_cast<int>(point.x / m_heightCellSize);
    index.y = static_cast<int>(point.y / m_heightCellSize);
    return index * (1 / m_scaleHoriz);
}

// TODO: precalculated data support
void Terrain::updateHeightMatrix()
{
    m_horizontalCellCount = ceil(heightmapWidth / m_heightCellSize);
    m_verticalCellCount = ceil(heightmapHeight / m_heightCellSize);

    std::cout << "horizontalCellCount: " << m_horizontalCellCount << ", verticalCellCount: " << m_verticalCellCount << std::endl;

    m_heightMatrix.clear();
    m_heightMatrix.resize(m_horizontalCellCount, std::vector<HeightCell>(m_verticalCellCount, HeightCell(0.0f, 0.0f)));

    // TODO: move as debug
    m_heightMatrixCulled.clear();
    m_heightMatrixCulled.resize(m_horizontalCellCount, std::vector<bool>(m_verticalCellCount, false));

    for (int i = 0; i < heightmapWidth; i++)
    {
        for (int j = 0; j < heightmapHeight; j++)
        {
            int horizontalCellIndex = (float)j / m_heightCellSize;
            int verticalCellIndex = (float)i / m_heightCellSize;

            float height = data[j * (int)heightmapWidth + i];
            HeightCell &cell = m_heightMatrix[verticalCellIndex][horizontalCellIndex];

            // check if first
            bool firstHorizontal = fmod(i, m_heightCellSize) == 0.f;
            bool firstVertical = fmod(j, m_heightCellSize) == 0.f;

            if (firstHorizontal && firstVertical)
            {
                cell.min = height;
                cell.max = height;
            }
            else if (height < cell.min)
                cell.min = height;
            else if (height > cell.max)
                cell.max = height;
        }
    }
}

void Terrain::calculatePlanes(glm::mat4 projMatrix, glm::mat4 viewMatrix)
{
    glm::mat4 mat = projMatrix * viewMatrix;

    for (int i = 4; i--;)
        m_planes[0][i] = mat[i][3] + mat[i][0]; // left
    for (int i = 4; i--;)
        m_planes[1][i] = mat[i][3] - mat[i][0]; // right
    for (int i = 4; i--;)
        m_planes[2][i] = mat[i][3] + mat[i][1]; // bottom
    for (int i = 4; i--;)
        m_planes[3][i] = mat[i][3] - mat[i][1]; // top
    for (int i = 4; i--;)
        m_planes[4][i] = mat[i][3] + mat[i][2]; // near
    // for (int i = 4; i--;)
    //     m_planes[5][i] = mat[i][3] - mat[i][2]; // far
    for (int i = 5; i--;)
        m_planes[i] = glm::normalize(m_planes[i]);
}
