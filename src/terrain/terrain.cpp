#include "terrain.h"

#include "../external/stb_image/stb_image.h"

// TODO: change asset path at runtime
Terrain::Terrain(PhysicsWorld *physicsWorld)
{
    // TODO: keep track initialization state
    resolution = 64;
    wireframe = false;
    terrainCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    level = 9;
    scaleFactor = 255.0f;
    fogMaxDist = 6600.0f;
    fogMinDist = 1000.0f;
    fogColor = glm::vec4(0.46f, 0.71f, 0.98f, 1.0f);
    uvOffset = glm::vec2(0.0f, 0.0f);
    alphaOffset = glm::vec2(0.0f, 0.0f);
    oneOverWidth = 1.5f;
    shadowBias = glm::vec3(0.020, 0.023, 0.005);
    showCascade = false;

    // int n = 255;
    int m = resolution; // m = (n+1)/4

    // create mxm
    unsigned int vbo_mxm, ebo_mxm;
    createMesh(m, m, vbo_mxm, vao_mxm, ebo_mxm);
    // TODO: 1 vao for mx3 and 3mx - rotate
    // create mx3
    unsigned int vbo_mx3, ebo_mx3;
    createMesh(m, 3, vbo_mx3, vao_mx3, ebo_mx3);
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

    // 3x3 - finer center
    unsigned int vbo_3x3, ebo_3x3;
    createMesh(3, 3, vbo_3x3, vao_3x3, ebo_3x3);
    // 2x2 - outside of terrain
    unsigned int vbo_2x2, ebo_2x2;
    createMesh(2, 2, vbo_2x2, vao_2x2, ebo_2x2);

    // buffer elevationSampler texture
    glGenTextures(1, &textureID);
    int nrComponents;
    data = stbi_loadf("assets/images/4096x4096.png", &width, &height, &nrComponents, 1);
    // data = stbi_loadf("assets/images/vehicle.png", &width, &height, &nrComponents, 1);
    if (data == nullptr)
    {
        fprintf(stderr, "Failed to read heightmap\n");
        return;
    }

    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "nrComponents: " << nrComponents << std::endl;

    GLenum format;
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Physics
    terrainBody = physicsWorld->createTerrain(
        width,
        height,
        data,
        0,
        1,
        1,
        false);

    // TODO: bound to terrain scale factor
    terrainBody->getWorldTransform().setOrigin(btVector3(width / 2, scaleFactor / 2, height / 2));
    terrainBody->getCollisionShape()->setLocalScaling(btVector3(1, scaleFactor, 1));

    // buffer normalMapSampler texture
    glGenTextures(1, &ntextureID);
    int nwidth, nheight, nnrComponents;
    unsigned char *ndata = stbi_load("assets/images/4096x4096-normal.png", &nwidth, &nheight, &nnrComponents, 0);
    if (ndata == nullptr)
    {
        fprintf(stderr, "Failed to read heightmap\n");
        return;
    }

    std::cout << "nwidth: " << nwidth << std::endl;
    std::cout << "nheight: " << nheight << std::endl;
    std::cout << "nnrComponents: " << nnrComponents << std::endl;

    GLenum nformat;
    if (nnrComponents == 1)
        nformat = GL_RED;
    else if (nnrComponents == 3)
        nformat = GL_RGB;
    else if (nnrComponents == 4)
        nformat = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, ntextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, nformat, nwidth, nheight, 0, nformat, GL_UNSIGNED_BYTE, ndata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(ndata);

    glGenTextures(1, &ttextureID);
    int twidth, theight, tnrComponents;
    int nrTextures = 6;

    unsigned char *tdata0 = stbi_load("assets/images/water-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata1 = stbi_load("assets/images/sand-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata2 = stbi_load("assets/images/stone-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata3 = stbi_load("assets/images/grass-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata4 = stbi_load("assets/images/rock-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata5 = stbi_load("assets/images/snow-1.jpg", &twidth, &theight, &tnrComponents, 0);

    if (tdata0 == nullptr || tdata1 == nullptr || tdata2 == nullptr || tdata3 == nullptr || tdata4 == nullptr || tdata5 == nullptr)
    {
        fprintf(stderr, "Failed to read textures\n");
        return;
    }

    std::cout << "twidth: " << twidth << std::endl;
    std::cout << "theight: " << theight << std::endl;
    std::cout << "tnrComponents: " << tnrComponents << std::endl;

    GLenum tformat;
    if (tnrComponents == 1)
        tformat = GL_RED;
    else if (tnrComponents == 3)
        tformat = GL_RGB;
    else if (tnrComponents == 4)
        tformat = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D_ARRAY, ttextureID);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8, twidth, theight, nrTextures, 0, tformat, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata0);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata1);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata2);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata3);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata4);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata5);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    stbi_image_free(tdata0);
    stbi_image_free(tdata1);
    stbi_image_free(tdata2);
    stbi_image_free(tdata3);
    stbi_image_free(tdata4);
    stbi_image_free(tdata5);

    w = 1.0 / width;
    h = 1.0 / height;

    std::cout << "w: " << w << std::endl;
    std::cout << "h: " << h << std::endl;
}

Terrain::~Terrain()
{
    stbi_image_free(data);
}

// https://stackoverflow.com/a/9194117/11601515
// multiple is a power of 2
int Terrain::roundUp(int numToRound, int multiple)
{
    assert(multiple && ((multiple & (multiple - 1)) == 0));
    return (numToRound + multiple - 1) & -multiple;
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

void Terrain::drawColor(Shader terrainShader, glm::vec3 cameraPosition, glm::vec3 lightPosition, glm::vec3 lightColor, float lightPower,
                        glm::mat4 view, glm::mat4 projection, GLuint shadowmapId, glm::vec3 camPos, glm::vec3 camView, glm::vec4 frustumDistances)
{
    terrainShader.use();
    terrainShader.setFloat("zscaleFactor", scaleFactor);
    terrainShader.setVec3("viewerPos", cameraPosition);
    terrainShader.setFloat("oneOverWidth", oneOverWidth);
    terrainShader.setVec2("alphaOffset", alphaOffset);
    terrainShader.setVec3("lightDirection", lightPosition);
    terrainShader.setVec3("lightColor", lightColor);
    terrainShader.setFloat("lightPower", lightPower);
    terrainShader.setVec2("uvOffset", uvOffset);
    terrainShader.setVec2("terrainSize", glm::vec2(width, height));
    terrainShader.setFloat("fogMaxDist", fogMaxDist);
    terrainShader.setFloat("fogMinDist", fogMinDist);
    terrainShader.setVec4("fogColor", fogColor);

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
    glUniform1i(glGetUniformLocation(terrainShader.id, "normalMapSampler"), 1);
    glBindTexture(GL_TEXTURE_2D, ntextureID);

    glActiveTexture(GL_TEXTURE0 + 2);
    glUniform1i(glGetUniformLocation(terrainShader.id, "textureSampler"), 2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, ttextureID);

    glActiveTexture(GL_TEXTURE0 + 3);
    glUniform1i(glGetUniformLocation(terrainShader.id, "ShadowMap"), 3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmapId);

    this->draw(terrainShader, cameraPosition);
}

// TODO: culling with furstum light aabb
void Terrain::drawDepth(Shader terrainShadow, glm::vec3 cameraPosition, glm::mat4 view, glm::mat4 projection)
{
    terrainShadow.use();
    terrainShadow.setFloat("zscaleFactor", scaleFactor);
    terrainShadow.setVec3("viewerPos", cameraPosition);
    terrainShadow.setFloat("oneOverWidth", oneOverWidth);
    terrainShadow.setVec2("alphaOffset", alphaOffset);
    terrainShadow.setVec2("uvOffset", uvOffset);
    terrainShadow.setVec2("terrainSize", glm::vec2(width, height));

    terrainShadow.setMat4("worldViewProjMatrix", projection * view);
    terrainShadow.setMat4("M", glm::mat4(1.0f));
    terrainShadow.setMat4("V", view);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(terrainShadow.id, "elevationSampler"), 0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    this->draw(terrainShadow, cameraPosition);
}

void Terrain::draw(Shader terrainShader, glm::vec3 viewerPos)
{
    int m = resolution;

    // for each level
    int lastRoundX, lastRoundZ = 0;
    for (int i = 1; i < level; i++)
    {
        // set param for each footprint
        int scale = pow(2, i - 1);

        int X = -1 * (2 * m * scale) + (int)viewerPos.x + (int)terrainCenter.x;
        int Z = -1 * (2 * m * scale) + (int)viewerPos.z + (int)terrainCenter.z;

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

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, x, z));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * z));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * z));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * z));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 2 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 3 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 3 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z + sizeMM * 3 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * (z + sizeMM * 3 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 3 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 3 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 3 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 3 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 2 + size2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 2 + size2)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM)));
        glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

        // fine level mxm
        if (i == 1)
        {
            terrainShader.setVec3("wireColor", glm::vec3(1, 1, 1));

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM + size2, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM + size2), h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + 2 * sizeMM + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + 2 * sizeMM + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM + size2, z + 2 * sizeMM + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM + size2), h * (z + 2 * sizeMM + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

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

        // fine level 3xm
        if (i == 1)
        {
            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);
        }

        // mx3
        glBindVertexArray(vao_mx3);
        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 2)));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 2));
        terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2)));
        glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

        if (i == 1)
        {
            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z + sizeMM * 2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * (z + sizeMM * 2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);
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
