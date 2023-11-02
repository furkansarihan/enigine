#ifndef shader_manager_hpp
#define shader_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

#include "../shader/shader.h"
#include "../file_manager/file_manager.h"

struct ShaderDynamic
{
    Shader *m_shader;
    std::string m_vsPath;
    std::string m_fsPath;

    ShaderDynamic(Shader *shader, std::string vsPath, std::string fsPath)
        : m_shader(shader),
          m_vsPath(vsPath),
          m_fsPath(fsPath)
    {
    }
};

class ShaderManager
{
public:
    ShaderManager(std::string executablePath);
    ~ShaderManager();

    std::string m_executablePath;
    std::vector<ShaderDynamic> m_shaderList;

    void addShader(const ShaderDynamic &shaderDynamic);
    void initShaders();
    void initShader(const ShaderDynamic &shaderDynamic);
    // TODO: listen file changes on dev mode

private:
    std::string processIncludes(const std::string &directory, const std::string &input);
};

#endif /* shader_manager_hpp */
