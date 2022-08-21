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
