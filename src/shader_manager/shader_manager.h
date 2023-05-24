#ifndef shader_manager_hpp
#define shader_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

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
    ShaderManager();
    ~ShaderManager();

    std::vector<ShaderDynamic> m_shaderList;

    void addShader(const ShaderDynamic &shaderDynamic);
    void initShaders();
    // TODO: listen file changes on dev mode
};

#endif /* shader_manager_hpp */
