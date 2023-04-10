#ifndef shader_hpp
#define shader_hpp

#include <string>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader();
    ~Shader();
    unsigned int id;
    void init(const std::string &vertexCode, const std::string &fragmentCode);
    void init(const std::string &vertexCode, const std::string &fragmentCode,
              const std::string &tessControlCode, const std::string &tessEvalCode);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void checkCompileError(unsigned int shader, std::string type);
    void checkLinkingError();
    void compile();
    void link();
    unsigned int vertexId_, fragmentId_, tessControlId_, tessEvalId_;
    std::string vertexCode_;
    std::string fragmentCode_;
    std::string tessControlCode_;
    std::string tessEvalCode_;
};

#endif /* shader_hpp */
