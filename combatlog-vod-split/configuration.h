#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <string>

class configuration
{
    public:
        
        std::filesystem::path exePath;
        std::string videoDirectory;
        std::string logDirectory;
        std::string mode;
        bool post_processing_protection;
        bool configFound;
        configuration();

        //to be depricated
        std::string video_directory;
        std::string log_directory;
};