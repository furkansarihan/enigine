#include "mesh.h"

Mesh::Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material material)
    : name(name),
      vertices(vertices),
      indices(indices),
      material(material)
{
    setupMesh();
    updateTransmission();
}

Mesh::~Mesh()
{
}

void Mesh::draw(Shader shader)
{
    bindTextures(shader);
    bindProperties(shader);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    unbindTextures(shader);
    unbindProperties(shader);
}

void Mesh::drawInstanced(Shader shader, int instanceCount)
{
    bindTextures(shader);
    bindProperties(shader);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);

    unbindTextures(shader);
    unbindProperties(shader);
}

void Mesh::bindTextures(Shader shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int roughNr = 1;
    unsigned int aoNr = 1;
    unsigned int metalNr = 1;
    unsigned int unknownNr = 1;
    for (unsigned int i = 0; i < material.textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = material.textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);
        else if (name == "texture_rough")
            number = std::to_string(roughNr++);
        else if (name == "texture_ao")
            number = std::to_string(aoNr++);
        else if (name == "texture_metal")
            number = std::to_string(metalNr++);
        else if (name == "texture_unknown")
            number = std::to_string(unknownNr++);

        glUniform1i(glGetUniformLocation(shader.id, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, material.textures[i].id);
    }
}

void Mesh::unbindTextures(Shader shader)
{
    for (unsigned int i = 0; i < material.textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Mesh::bindProperties(Shader shader)
{
    for (int i = 0; i < material.properties.size(); i++)
    {
        MaterialProperty &property = material.properties[i];

        // TODO: better way without casting?
        if (property.type == aiPTI_Float)
        {
            float value = std::stof(property.value);
            shader.setFloat(property.name, value);
        }
        else if (property.type == aiPTI_Integer)
        {
            int value = std::stoi(property.value);
            shader.setInt(property.name, value);
        }
    }
}

void Mesh::unbindProperties(Shader shader)
{
    for (int i = 0; i < material.properties.size(); i++)
    {
        MaterialProperty &property = material.properties[i];

        if (property.type == aiPTI_Float)
            shader.setFloat(property.name, 0.f);
        else if (property.type == aiPTI_Integer)
            shader.setInt(property.name, 0);
    }
}

void Mesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));
    // boneIDs - for animation
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, boneIDs));
    // weights - for animation
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, weights));

    glBindVertexArray(0);
}

void Mesh::updateTransmission()
{
    for (int i = 0; i < material.properties.size(); i++)
    {
        MaterialProperty &property = material.properties[i];
        if (property.name == "transmission_factor")
        {
            float value = std::stof(property.value);
            if (value > 0.f)
            {
                opaque = false;
                return;
            }
        }
    }
}
