#include "shader_manager.h"

ShaderManager::ShaderManager(std::string executablePath)
    : m_executablePath(executablePath)
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::addShader(const ShaderDynamic &shaderDynamic)
{
    std::string vsPath = m_executablePath + '/' + shaderDynamic.m_vsPath;
    std::string fsPath = m_executablePath + '/' + shaderDynamic.m_fsPath;
    shaderDynamic.m_shader->init(FileManager::read(vsPath), FileManager::read(fsPath));
    m_shaderList.push_back(shaderDynamic);
}

void ShaderManager::initShaders()
{
    for (int i = 0; i < m_shaderList.size(); i++)
    {
        ShaderDynamic sd = m_shaderList.at(i);
        std::string vsPath = m_executablePath + '/' + sd.m_vsPath;
        std::string fsPath = m_executablePath + '/' + sd.m_fsPath;
        sd.m_shader->init(FileManager::read(vsPath), FileManager::read(fsPath));
    }
}
