#include "debug_drawer.h"

DebugDrawer::DebugDrawer()
{
}

DebugDrawer::~DebugDrawer()
{
}

void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    glm::vec3 src(from.x(), from.y(), from.z());
    glm::vec3 dst(to.x(), to.y(), to.z());
    glm::vec3 col(color.x(), color.y(), color.z());

    Line l(src, dst, col);
    this->lines.push_back(l);
}

// TODO: own shader with only lines
void DebugDrawer::drawLines(Shader &lineShader, glm::mat4 mvp, unsigned int vbo, unsigned int vao, unsigned int ebo)
{
    if (this->lines.size() == 0)
        return;

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    unsigned int indexI = 0;

    for (std::vector<DebugDrawer::Line>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        DebugDrawer::Line l = (*it);
        vertices.push_back(l.a.x);
        vertices.push_back(l.a.y);
        vertices.push_back(l.a.z);

        vertices.push_back(l.color.x);
        vertices.push_back(l.color.y);
        vertices.push_back(l.color.z);

        vertices.push_back(l.b.x);
        vertices.push_back(l.b.y);
        vertices.push_back(l.b.z);

        vertices.push_back(l.color.x);
        vertices.push_back(l.color.y);
        vertices.push_back(l.color.z);

        indices.push_back(indexI);
        indices.push_back(indexI + 1);
        indexI += 2;
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    lineShader.use();
    lineShader.setMat4("MVP", mvp);

    glBindVertexArray(vao);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
