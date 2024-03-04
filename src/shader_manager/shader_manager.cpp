#include "shader_manager.h"

#include <filesystem>
#include <iostream>

ShaderManager::ShaderManager(std::string executablePath)
    : m_executablePath(executablePath)
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::addShader(const ShaderDynamic &shaderDynamic)
{
    m_shaderList.push_back(shaderDynamic);
    initShader(shaderDynamic);
}

void ShaderManager::initShaders()
{
    for (int i = 0; i < m_shaderList.size(); i++)
    {
        ShaderDynamic sd = m_shaderList.at(i);
        initShader(sd);
    }
}

void ShaderManager::initShader(const ShaderDynamic &shaderDynamic)
{
    std::filesystem::path vsPath = m_executablePath + '/' + shaderDynamic.m_vsPath;
    std::filesystem::path fsPath = m_executablePath + '/' + shaderDynamic.m_fsPath;
    std::string vsPathStr = std::string(m_executablePath + '/' + shaderDynamic.m_vsPath);
    std::string fsPathStr = std::string(m_executablePath + '/' + shaderDynamic.m_fsPath);
    std::cout << vsPathStr << std::endl;
    std::string vsCode = FileManager::read(vsPathStr);
    std::string fsCode = FileManager::read(fsPathStr);
    std::string vsDirectory = vsPath.parent_path().string();
    std::string fsDirectory = fsPath.parent_path().string();
    shaderDynamic.m_shader->init(processIncludes(vsDirectory, vsCode), processIncludes(fsDirectory, fsCode));
}

// Function to process and replace #include directives
std::string ShaderManager::processIncludes(const std::string &directory, const std::string &input)
{
    std::regex includeRegex("#include\\s*<([^>]*)>");
    std::string result;

    std::smatch match;
    std::string remainingInput = input; // Use a separate string for remaining input

    while (std::regex_search(remainingInput, match, includeRegex))
    {
        result += match.prefix(); // Append text before the match
        std::string includeFile = match[1];
        std::string includePath = directory + '/' + includeFile;
        std::string includedContent = FileManager::read(includePath); // Use FileManager::read
        if (!includedContent.empty())
        {
            result += includedContent;
        }
        remainingInput = match.suffix(); // Update remainingInput
    }

    result += remainingInput; // Append the remaining text
    return result;
}
