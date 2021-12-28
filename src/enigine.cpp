#include <iostream>
#include <ctime>

int main(int argc, char **argv)
{
    // current date/time based on current system
    time_t now = time(0);
    char *dateTime = ctime(&now);
    // version, format -> x.xx.xxx
    std::string version ("000001");

    std::cout << std::endl << "enigine" << std::endl << std::endl;
    std::cout << "enigine_version: " << version << std::endl;
    std::cout << "cpp_version: " << __cplusplus << std::endl;
    std::cout << "started_at: " << dateTime << std::endl;
    return 0;
}