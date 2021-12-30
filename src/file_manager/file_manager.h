#ifndef file_manager_hpp
#define file_manager_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

class FileManager
{
public:
    FileManager();
    ~FileManager();
    static std::string read(const std::string &filename);
};

#endif /* file_manager_hpp */
