#include <windows.h>
#include <iostream>
#include <shlobj.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream> 
#include <sstream>
#include <vector>

#include "configuration.h"


//gets directory of executable which is where the the config file is stored
std::filesystem::path GetExeDirectory()
{
    wchar_t szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);
    return std::filesystem::path { szPath }.parent_path();
};

configuration::configuration()
{
    //gets the exe path to get the config file
    this->exePath = GetExeDirectory();

    //checks if there is a config file within the executable's directory
    //if it is not there generates a sample config
    if (!std::filesystem::exists(exePath.string() + "\\config.conf"))
    {
        std::ofstream outfile(exePath.string() + "\\config.conf");
        outfile << "video_directory=\nlog_directory=\nmode=";
        outfile.close();
        std::cout << "Configuration file not found please configure and restart the application" << std::endl;
        this->configFound = false;
    }
    else
    {
        std::string line;
        std::ifstream file(exePath.string() + "\\config.conf");

        //reads config file and sets values locally in the program
        while (std::getline(file, line))
        {
            try
            {
                int found = line.find("=");
                if (found > 1 && found != line.size())
                {
                    //if I wanted to add more settings it should be in here but idk what else
                    //people would want to have as a setting
                    //maybe something for different games if I find ways to parse other games logs
                    //or maybe speed runner stuff?
                    std::string setting = line.substr(0, found);
                    if (setting == "video_directory")
                    {
                        this->videoDirectory = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "log_directory")
                    {
                        this->logDirectory = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "mode")
                    {
                        this->mode = line.substr(found + 1, line.size() - 1);
                    }
                }
            }
            catch (const std::runtime_error& error)
            {
                std::cout << error.what() << '\n';
            }
        }

        //checks for missing settings
        if (videoDirectory == "")
        {
            std::cout << "Error setting video directory - please check config file" << std::endl;
            this->configFound = false;
        }
        if (logDirectory == "")
        {
            std::cout << "Error setting log directory - please check config file" << std::endl;
            this->configFound = false;
        }
        if (mode == "")
        {
            std::cout << "Error setting mode - please check config file" << std::endl;
            this->configFound = false;
        }

        this->configFound = true;
    }
};