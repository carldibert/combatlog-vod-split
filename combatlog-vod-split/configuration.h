#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <string>

class configuration
{
    public:
        
        std::filesystem::path exePath;
        std::string video_directory;
        std::string log_directory;
        std::string mode;
        bool configFound;
        configuration();
};