#include "shader.h"

Shader::Shader()
{
}

Shader::~Shader()
{
}

void Shader::init(const std::string &vertexCode, const std::string &fragmentCode)
{
    vertexCode_ = vertexCode;
    fragmentCode_ = fragmentCode;
    compile();
    link();
}

void Shader::init(const std::string &vertexCode, const std::string &fragmentCode,
                  const std::string &tessControlCode, const std::string &tessEvalCode)
{
    vertexCode_ = vertexCode;
    fragmentCode_ = fragmentCode;
    tessControlCode_ = tessControlCode;
    tessEvalCode_ = tessEvalCode;
    compile();
    link();
}

void Shader::compile()
{
    const char *vcode = vertexCode_.c_str();
    vertexId_ = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexId_, 1, &vcode, NULL);
    glCompileShader(vertexId_);
    checkCompileError(vertexId_, "VERTEX");

    const char *fcode = fragmentCode_.c_str();
    fragmentId_ = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentId_, 1, &fcode, NULL);
    glCompileShader(fragmentId_);
    checkCompileError(fragmentId_, "FRAGMENT");

    if (!tessControlCode_.empty())
    {
        const char *tcShaderCode = tessControlCode_.c_str();
        tessControlId_ = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tessControlId_, 1, &tcShaderCode, NULL);
        glCompileShader(tessControlId_);
        checkCompileError(tessControlId_, "TESS_CONTROL");
    }

    if (!tessEvalCode_.empty())
    {
        const char *teShaderCode = tessEvalCode_.c_str();
        tessEvalId_ = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tessEvalId_, 1, &teShaderCode, NULL);
        glCompileShader(tessEvalId_);
        checkCompileError(tessEvalId_, "TESS_EVALUATION");
    }
}

void Shader::link()
{
    id = glCreateProgram();
    glAttachShader(id, vertexId_);
    glAttachShader(id, fragmentId_);
    if (!tessControlCode_.empty())
    {
        glAttachShader(id, tessControlId_);
    }
    if (!tessEvalCode_.empty())
    {
        glAttachShader(id, tessEvalId_);
    }
    glLinkProgram(id);
    checkLinkingError();
    glDeleteShader(vertexId_);
    glDeleteShader(fragmentId_);
    if (!tessControlCode_.empty())
        glDeleteShader(tessControlId_);
    if (!tessEvalCode_.empty())
        glDeleteShader(tessEvalId_);
}

void Shader::use()
{
    glUseProgram(id);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string &name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileError(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "Shader: Error compiling : " << type << ":\n"
                  << infoLog << std::endl;
    }
}

void Shader::checkLinkingError()
{
    int success;
    char infoLog[1024];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 1024, NULL, infoLog);
        std::cout << "Shader: Error Linking Shader Program:\n"
                  << infoLog << std::endl;
    }
}
