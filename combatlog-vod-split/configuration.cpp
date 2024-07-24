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

//for once something that actually was thought out
//initalizes the configuration to the user's documents directory
//this is where the DLLs and executable should be stored
//but they can put it wherever but the config needs to be there to read from unless I let it store in the .exe
//ill need to ask people what they think idk im not their real dad
void configuration::SetConfiguration()
{
    //gathers document folder location
    TCHAR szPath[MAX_PATH];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath);
    std::wstring arr_w(szPath);
    std::string tmp(arr_w.begin(), arr_w.end());
    this->my_documents = tmp;
};

//checks for a configuration file
bool configuration::CheckForConfigFile()
{
    //prolly shouldnt have this since I am going to be packaging this in one directory
    //checks if combat_log_vod_split directory exists and create it does not exist
    std::string configDirectory = my_documents + "\\combat_log_vod_split";
    std::filesystem::path tmp = configDirectory;
    CreateDirectory(tmp.c_str(), NULL);

    //checks to see if configuration file is missing
    //if missing it creates a blank config file and should prolly include one with some defaults either in the readme or by default
    //I should also really look at where the default log location is
    if (!std::filesystem::exists(configDirectory + "\\config.conf"))
    {
        std::ofstream outfile(configDirectory + "\\config.conf");
        outfile << "video_directory=\nlog_directory=\nmode=";
        outfile.close();
        std::cout << "Configuration file not found please configure" << std::endl;
        return false;
    }
    else
    {
        std::string line;
        std::ifstream file(configDirectory + "\\config.conf");

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
                        this->video_directory = line.substr(found + 1, line.size() - 1);
                    }
                    else if (setting == "log_directory")
                    {
                        this->log_directory = line.substr(found + 1, line.size() - 1);
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
        if (video_directory == "")
        {
            std::cout << "Error setting video directory - please check config file" << std::endl;
            return false;
        }
        if (log_directory == "")
        {
            std::cout << "Error setting log directory - please check config file" << std::endl;
            return false;
        }
        if (mode == "")
        {
            std::cout << "Error setting mode - please check config file" << std::endl;
            return false;
        }
    }

    //config file has loaded everything properly
    return true;
};