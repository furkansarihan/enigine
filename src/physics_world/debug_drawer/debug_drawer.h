#ifndef debug_drawer_hpp
#define debug_drawer_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "btBulletDynamicsCommon.h"

class DebugDrawer : public btIDebugDraw
{

public:
    DebugDrawer();
    ~DebugDrawer();

    struct Line
    {
        Line(glm::vec3 a, glm::vec3 b, glm::vec3 color) : a(a), b(b), color(color){};

        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 color;
    };

    virtual void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
    virtual void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) {}

    void reportErrorWarning(const char *warningString) { std::cout << "Physics debugger warning: " << warningString << std::endl; }

    virtual void draw3dText(const btVector3 &location, const char *textString) {}
    virtual void setDebugMode(int debugMode) { m_debugMode = debugMode; }
    virtual int getDebugMode() const { return m_debugMode; }

    std::vector<Line> &getLines() { return lines; }

private:
    int m_debugMode = 1;
    std::vector<Line> lines;
};
#endif /* debug_drawer_hpp */
