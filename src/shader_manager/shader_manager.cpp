#include "shader_manager.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::addShader(const ShaderDynamic &shaderDynamic)
{
    shaderDynamic.m_shader->init(FileManager::read(shaderDynamic.m_vsPath), FileManager::read(shaderDynamic.m_fsPath));
    m_shaderList.push_back(shaderDynamic);
}

void ShaderManager::initShaders()
{
    for (int i = 0; i < m_shaderList.size(); i++)
    {
        ShaderDynamic sd = m_shaderList.at(i);
        sd.m_shader->init(FileManager::read(sd.m_vsPath), FileManager::read(sd.m_fsPath));
    }
}
