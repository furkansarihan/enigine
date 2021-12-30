#include "file_manager.h"

FileManager::FileManager()
{
}

FileManager::~FileManager()
{
}

std::string FileManager::read(const std::string &filename)
{
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::stringstream fileStream;
    try
    {
        file.open(filename.c_str());
        fileStream << file.rdbuf();
        file.close();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "Error reading Shader File!" << std::endl;
    }
    return fileStream.str();
}
