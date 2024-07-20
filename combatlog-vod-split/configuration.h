#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <string>

class configuration
{
    public:
        std::string my_documents;
        std::string video_directory;
        std::string log_directory;
        std::string mode;
        int threads;
        void SetConfiguration();
        bool CheckForConfigFile();
};